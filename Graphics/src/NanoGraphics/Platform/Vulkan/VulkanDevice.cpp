#include "ngpch.h"
#include "VulkanDevice.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

namespace Nano::Graphics::Internal
{

    static_assert(std::is_same_v<Image::Type, VulkanImage>, "Current Image::Type is not VulkanImage and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<Sampler::Type, VulkanSampler>, "Current Sampler::Type is not VulkanSampler and Vulkan source code is being compiled.");

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanDevice::VulkanDevice(const DeviceSpecification& specs)
        : m_Context(specs.NativeWindow, specs.MessageCallback, specs.DestroyCallback, specs.Extensions), m_Allocator(m_Context.GetVkInstance(), m_Context.GetVulkanPhysicalDevice().GetVkPhysicalDevice(), m_Context.GetVulkanLogicalDevice().GetVkDevice())
    {
    }

    VulkanDevice::~VulkanDevice()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanDevice::Wait() const
    {
        m_Context.GetVulkanLogicalDevice().Wait();
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Destruction methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanDevice::DestroySwapchain(Swapchain& swapchain) const
    {
        // TODO: ...
    }

    void VulkanDevice::DestroyImage(Image& image) const
    {
        DestroySubresourceViews(image);

        VulkanImage& vulkanImage = *reinterpret_cast<VulkanImage*>(&image);
        VkImage vkImage = vulkanImage.GetVkImage();
        VmaAllocation allocation = vulkanImage.GetVmaAllocation();
        m_Context.Destroy([vkImage, allocation, allocator = &m_Allocator]() mutable
        {
            allocator->DestroyImage(vkImage, allocation);
        });
    }

    void VulkanDevice::DestroySubresourceViews(Image& image) const
    {
        VulkanImage& vkImage = *reinterpret_cast<VulkanImage*>(&image);

        std::vector<VkImageView> imageViews;
        imageViews.reserve(vkImage.GetImageViews().size());
        for (const auto& [_, view] : vkImage.GetImageViews())
            imageViews.push_back(view.GetVkImageView());

        VkDevice device = m_Context.GetVulkanLogicalDevice().GetVkDevice();
        m_Context.Destroy([device, imageViews = std::move(imageViews)]() mutable
        {
            for (const auto& iview : imageViews)
                vkDestroyImageView(device, iview, VulkanAllocator::GetCallbacks());
        });
    }

    void VulkanDevice::DestroySampler(Sampler& sampler) const
    {
        VulkanSampler& vulkanSampler = *reinterpret_cast<VulkanSampler*>(&sampler);

        VkDevice device = m_Context.GetVulkanLogicalDevice().GetVkDevice();
        VkSampler vkSampler = vulkanSampler.GetVkSampler();
        m_Context.Destroy([device, vkSampler]() mutable
        {
            vkDestroySampler(device, vkSampler, VulkanAllocator::GetCallbacks());
        });

    }

}