#include "ngpch.h"
#include "VulkanDevice.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanImage.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanSwapchain.hpp"

namespace Nano::Graphics::Internal
{

    static_assert(std::is_same_v<Swapchain::Type, VulkanSwapchain>, "Current Swapchain::Type is not VulkanSwapchain and Vulkan source code is being compiled.");
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
        VulkanSwapchain& vulkanSwapchain = *reinterpret_cast<VulkanSwapchain*>(&swapchain);

        for (auto& image : vulkanSwapchain.m_Images)
            DestroySubresourceViews(*reinterpret_cast<Image*>(&image.Get()));

        m_Context.Destroy([instance = m_Context.GetVkInstance(), device = m_Context.GetVulkanLogicalDevice().GetVkDevice(), swapchain = vulkanSwapchain.m_Swapchain, surface = vulkanSwapchain.m_Surface, imageSemaphores = vulkanSwapchain.m_ImageAvailableSemaphores, swapchainPresentableSemaphores = vulkanSwapchain.m_SwapchainPresentableSemaphores,  timelineSemaphore = vulkanSwapchain.m_TimelineSemaphore]() mutable
        {
            vkDestroySwapchainKHR(device, swapchain, VulkanAllocator::GetCallbacks());
            vkDestroySurfaceKHR(instance, surface, VulkanAllocator::GetCallbacks());

            for (size_t i = 0; i < imageSemaphores.size(); i++)
            {
                vkDestroySemaphore(device, imageSemaphores[i], VulkanAllocator::GetCallbacks());
                vkDestroySemaphore(device, swapchainPresentableSemaphores[i], VulkanAllocator::GetCallbacks());
            }

            vkDestroySemaphore(device, timelineSemaphore, VulkanAllocator::GetCallbacks());
        });

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