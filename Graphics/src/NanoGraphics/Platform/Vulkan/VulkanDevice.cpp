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
        : m_Context(specs.NativeWindow, specs.MessageCallback, specs.Extensions), m_Allocator(m_Context.GetVkInstance(), m_Context.GetVulkanPhysicalDevice().GetVkPhysicalDevice(), m_Context.GetVulkanLogicalDevice().GetVkDevice())
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
    void VulkanDevice::FreePool(CommandListPool& pool) const
    {
        vkDestroyCommandPool(m_Context.GetVulkanLogicalDevice().GetVkDevice(), (*reinterpret_cast<VulkanCommandListPool*>(&pool)).GetVkCommandPool(), m_Allocator.GetCallbacks());
    }

    void VulkanDevice::DestroyImage(Image& image) const
    {
        VulkanImage& vkImage = *reinterpret_cast<VulkanImage*>(&image);
        DestroySubresourceViews(image);
        m_Allocator.DestroyImage(vkImage.GetVkImage(), vkImage.GetVmaAllocation());
    }

    void VulkanDevice::DestroySubresourceViews(Image& image) const
    {
        VulkanImage& vkImage = *reinterpret_cast<VulkanImage*>(&image);

        for (const auto& [key, view] : vkImage.GetImageViews())
            vkDestroyImageView(m_Context.GetVulkanLogicalDevice().GetVkDevice(), view.GetVkImageView(), m_Allocator.GetCallbacks());

        vkImage.GetImageViews().clear();
    }

    void VulkanDevice::DestroySampler(Sampler& sampler) const
    {
        VulkanSampler& vkSampler = *reinterpret_cast<VulkanSampler*>(&sampler);
        vkDestroySampler(m_Context.GetVulkanLogicalDevice().GetVkDevice(), vkSampler.GetVkSampler(), m_Allocator.GetCallbacks());
    }

    void VulkanDevice::ResetPool(CommandListPool& pool) const
    {
        vkResetCommandPool(m_Context.GetVulkanLogicalDevice().GetVkDevice(), (*reinterpret_cast<VulkanCommandListPool*>(&pool)).GetVkCommandPool(), 0);
    }

}