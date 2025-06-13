#include "ngpch.h"
#include "VulkanDevice.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/Swapchain.hpp"
#include "NanoGraphics/Renderer/Image.hpp"
#include "NanoGraphics/Renderer/Buffer.hpp"
#include "NanoGraphics/Renderer/Framebuffer.hpp"
#include "NanoGraphics/Renderer/Renderpass.hpp"
#include "NanoGraphics/Renderer/Shader.hpp"
#include "NanoGraphics/Renderer/Pipeline.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanImage.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanBuffer.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanSwapchain.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanRenderpass.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanShader.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanPipeline.hpp"

namespace Nano::Graphics::Internal
{

    static_assert(std::is_same_v<Swapchain::Type, VulkanSwapchain>, "Current Swapchain::Type is not VulkanSwapchain and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<Image::Type, VulkanImage>, "Current Image::Type is not VulkanImage and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<Buffer::Type, VulkanBuffer>, "Current Buffer::Type is not VulkanBuffer and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<Sampler::Type, VulkanSampler>, "Current Sampler::Type is not VulkanSampler and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<Framebuffer::Type, VulkanFramebuffer>, "Current Framebuffer::Type is not VulkanFramebuffer and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<Renderpass::Type, VulkanRenderpass>, "Current Renderpass::Type is not VulkanRenderpass and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<Shader::Type, VulkanShader>, "Current Shader::Type is not VulkanShader and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<GraphicsPipeline::Type, VulkanGraphicsPipeline>, "Current GraphicsPipeline::Type is not VulkanGraphicsPipeline and Vulkan source code is being compiled.");

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
            for (auto iview : imageViews)
                vkDestroyImageView(device, iview, VulkanAllocator::GetCallbacks());
        });

        vkImage.GetImageViews().clear();
    }

    void VulkanDevice::DestroyBuffer(Buffer& buffer) const
    {
        VulkanBuffer& vulkanBuffer = *reinterpret_cast<VulkanBuffer*>(&buffer);

        VkDevice device = m_Context.GetVulkanLogicalDevice().GetVkDevice();
        VkBuffer vkBuffer = vulkanBuffer.GetVkBuffer();
        VmaAllocation allocation = vulkanBuffer.GetVmaAllocation();
        m_Context.Destroy([device, vkBuffer, allocation, allocator = &m_Allocator]() mutable
        {
            NG_ASSERT(false, "TODO: Uncomment"); // TODO: ...
            //allocator->DestroyBuffer(vkBuffer, allocation);
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

    void VulkanDevice::DestroyFramebuffer(Framebuffer& framebuffer) const
    {
        VulkanFramebuffer& vulkanFramebuffer = *reinterpret_cast<VulkanFramebuffer*>(&framebuffer);

        VkDevice device = m_Context.GetVulkanLogicalDevice().GetVkDevice();
        VkFramebuffer vkFramebuffer = vulkanFramebuffer.GetVkFramebuffer();
        m_Context.Destroy([device, vkFramebuffer]() mutable
        {
            vkDestroyFramebuffer(device, vkFramebuffer, VulkanAllocator::GetCallbacks());
        });
    }

    void VulkanDevice::DestroyRenderpass(Renderpass& renderpass) const
    {
        VulkanRenderpass& vulkanRenderpass = *reinterpret_cast<VulkanRenderpass*>(&renderpass);

        VkDevice device = m_Context.GetVulkanLogicalDevice().GetVkDevice();
        VkRenderPass vkRenderpass = vulkanRenderpass.GetVkRenderPass();

        for (auto& framebuffer : vulkanRenderpass.GetVulkanFramebuffers())
            DestroyFramebuffer(*reinterpret_cast<Framebuffer*>(&framebuffer));

        m_Context.Destroy([device, vkRenderpass]() mutable
        {
            vkDestroyRenderPass(device, vkRenderpass, VulkanAllocator::GetCallbacks());
        });
    }

    void VulkanDevice::DestroyShader(Shader& shader) const
    {
        VulkanShader& vulkanShader = *reinterpret_cast<VulkanShader*>(&shader);

        VkDevice device = m_Context.GetVulkanLogicalDevice().GetVkDevice();
        VkShaderModule vkShader = vulkanShader.GetVkShaderModule();
        m_Context.Destroy([device, vkShader]() mutable
        {
            vkDestroyShaderModule(device, vkShader, VulkanAllocator::GetCallbacks());
        });
    }

    void VulkanDevice::DestroyGraphicsPipeline(GraphicsPipeline& pipeline) const
    {
        VulkanGraphicsPipeline& vulkanGraphicsPipeline = *reinterpret_cast<VulkanGraphicsPipeline*>(&pipeline);

        VkDevice device = m_Context.GetVulkanLogicalDevice().GetVkDevice();
        NG_ASSERT(false, "TODO");
        // TODO: ...
    }

}