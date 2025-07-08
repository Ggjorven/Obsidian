#include "ngpch.h"
#include "VulkanDevice.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/Bindings.hpp"
#include "NanoGraphics/Renderer/Swapchain.hpp"
#include "NanoGraphics/Renderer/Image.hpp"
#include "NanoGraphics/Renderer/Buffer.hpp"
#include "NanoGraphics/Renderer/Framebuffer.hpp"
#include "NanoGraphics/Renderer/Renderpass.hpp"
#include "NanoGraphics/Renderer/Shader.hpp"
#include "NanoGraphics/Renderer/Pipeline.hpp"

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanDevice::VulkanDevice(const DeviceSpecification& specs)
        : m_Context(specs.NativeWindow, specs.MessageCallback, specs.DestroyCallback, specs.Extensions), m_Allocator(m_Context.GetVkInstance(), m_Context.GetVulkanPhysicalDevice().GetVkPhysicalDevice(), m_Context.GetVulkanLogicalDevice().GetVkDevice()), m_StateTracker(*api_cast<const Device*>(this))
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

    void VulkanDevice::StartTracking(const Image& image, ImageSubresourceSpecification subresources, ResourceState currentState)
    {
        NG_PROFILE("VulkanDevice::StartTracking()");
        m_StateTracker.StartTracking(image, subresources, currentState);
    }

    void VulkanDevice::StartTracking(const StagingImage& image, ResourceState currentState)
    {
        NG_PROFILE("VulkanDevice::StartTracking()");
        const VulkanStagingImage& vulkanStagingImage = *api_cast<const VulkanStagingImage*>(&image);
        m_StateTracker.StartTracking(*api_cast<const Buffer*>(&vulkanStagingImage.GetVulkanBuffer()), currentState);
    }

    void VulkanDevice::StartTracking(const Buffer& buffer, ResourceState currentState)
    {
        NG_PROFILE("VulkanDevice::StartTracking()");
        m_StateTracker.StartTracking(buffer, currentState);
    }

    void VulkanDevice::StopTracking(const Image& image)
    {
        NG_PROFILE("VulkanDevice::StopTracking()");
        m_StateTracker.StopTracking(image);
    }

    void VulkanDevice::StopTracking(const StagingImage& image)
    {
        NG_PROFILE("VulkanDevice::StopTracking()");
        const VulkanStagingImage& vulkanStagingImage = *api_cast<const VulkanStagingImage*>(&image);
        m_StateTracker.StopTracking(*api_cast<const Buffer*>(&vulkanStagingImage.GetVulkanBuffer()));
    }

    void VulkanDevice::StopTracking(const Buffer& buffer)
    {
        NG_PROFILE("VulkanDevice::StopTracking()");
        m_StateTracker.StopTracking(buffer);
    }

    void VulkanDevice::MapBuffer(const Buffer& buffer, void*& memory) const
    {
        NG_PROFILE("VulkanDevice::MapBuffer()");
        const VulkanBuffer& vulkanBuffer = *api_cast<const VulkanBuffer*>(&buffer);
        NG_ASSERT(static_cast<bool>(buffer.GetSpecification().CpuAccess & CpuAccessMode::Write), "[VkDevice] Can't map buffer without CpuAccessMode::Write flag.");
        m_Allocator.MapMemory(vulkanBuffer.GetVmaAllocation(), memory);
    }

    void VulkanDevice::UnmapBuffer(const Buffer& buffer) const
    {
        NG_PROFILE("VulkanDevice::UnmapBuffer()");
        const VulkanBuffer& vulkanBuffer = *api_cast<const VulkanBuffer*>(&buffer);
        m_Allocator.UnmapMemory(vulkanBuffer.GetVmaAllocation());
    }

    void VulkanDevice::WriteBuffer(const Buffer& buffer, const void* memory, size_t size, size_t srcOffset, size_t dstOffset) const
    {
        NG_PROFILE("VulkanDevice::WriteBuffer()");

        NG_ASSERT((size + dstOffset <= buffer.GetSpecification().Size), "[VkDevice] Size + offset exceeds buffer size.");

        void* bufferMemory;
        MapBuffer(buffer, bufferMemory);

        std::memcpy(static_cast<uint8_t*>(bufferMemory) + dstOffset, static_cast<const uint8_t*>(memory) + srcOffset, size);

        UnmapBuffer(buffer);
    }

    void VulkanDevice::WriteImage(const StagingImage& image, const ImageSliceSpecification& slice, const void* memory, size_t size) const
    {
        NG_PROFILE("VulkanDevice::WriteImage()");
        const VulkanStagingImage& vkImage = *api_cast<const VulkanStagingImage*>(&image);
        const VulkanBuffer& vkBuffer = *api_cast<const VulkanBuffer*>(&vkImage.GetVulkanBuffer());
        const Buffer& buffer = *api_cast<const Buffer*>(&vkBuffer);

        VulkanStagingImage::Region region = vkImage.GetSliceRegion(slice.ImageMipLevel, slice.ImageArraySlice, 0);

        void* imageMemory;
        MapBuffer(buffer, imageMemory);

        std::memcpy(static_cast<uint8_t*>(imageMemory) + region.Offset, static_cast<const uint8_t*>(memory), size);

        UnmapBuffer(buffer);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Destruction methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanDevice::DestroySwapchain(Swapchain& swapchain) const
    {
        VulkanSwapchain& vulkanSwapchain = *api_cast<VulkanSwapchain*>(&swapchain);

        for (auto& image : vulkanSwapchain.m_Images)
            DestroySubresourceViews(*api_cast<Image*>(&image.Get()));

        m_Context.Destroy([instance = m_Context.GetVkInstance(), device = m_Context.GetVulkanLogicalDevice().GetVkDevice(), swapchain = vulkanSwapchain.m_Swapchain, surface = vulkanSwapchain.m_Surface, imageSemaphores = vulkanSwapchain.m_ImageAvailableSemaphores, swapchainPresentableSemaphores = vulkanSwapchain.m_SwapchainPresentableSemaphores,  timelineSemaphore = vulkanSwapchain.m_TimelineSemaphore, resizePool = vulkanSwapchain.m_ResizePool]() mutable
        {
            vkDestroyCommandPool(device, resizePool, VulkanAllocator::GetCallbacks());

            vkDestroySwapchainKHR(device, swapchain, VulkanAllocator::GetCallbacks());
            vkDestroySurfaceKHR(instance, surface, VulkanAllocator::GetCallbacks());

            for (size_t i = 0; i < imageSemaphores.size(); i++)
                vkDestroySemaphore(device, imageSemaphores[i], VulkanAllocator::GetCallbacks());
            for (size_t i = 0; i < swapchainPresentableSemaphores.size(); i++)
                vkDestroySemaphore(device, swapchainPresentableSemaphores[i], VulkanAllocator::GetCallbacks());

            vkDestroySemaphore(device, timelineSemaphore, VulkanAllocator::GetCallbacks());
        });

    }

    void VulkanDevice::DestroyImage(Image& image) const
    {
        DestroySubresourceViews(image);

        VulkanImage& vulkanImage = *api_cast<VulkanImage*>(&image);
        VkImage vkImage = vulkanImage.GetVkImage();
        VmaAllocation allocation = vulkanImage.GetVmaAllocation();
        m_Context.Destroy([vkImage, allocation, allocator = &m_Allocator]() mutable
        {
            allocator->DestroyImage(vkImage, allocation);
        });
    }

    void VulkanDevice::DestroySubresourceViews(Image& image) const
    {
        VulkanImage& vkImage = *api_cast<VulkanImage*>(&image);

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

    void VulkanDevice::DestroyStagingImage(StagingImage& stagingImage) const
    {
        VulkanStagingImage& vulkanStagingImage = *api_cast<VulkanStagingImage*>(&stagingImage);
        DestroyBuffer(*api_cast<Buffer*>(&vulkanStagingImage.GetVulkanBuffer()));
    }

    void VulkanDevice::DestroySampler(Sampler& sampler) const
    {
        VulkanSampler& vulkanSampler = *api_cast<VulkanSampler*>(&sampler);

        VkDevice device = m_Context.GetVulkanLogicalDevice().GetVkDevice();
        VkSampler vkSampler = vulkanSampler.GetVkSampler();
        m_Context.Destroy([device, vkSampler]() mutable
        {
            vkDestroySampler(device, vkSampler, VulkanAllocator::GetCallbacks());
        });
    }

    void VulkanDevice::DestroyBuffer(Buffer& buffer) const
    {
        VulkanBuffer& vulkanBuffer = *api_cast<VulkanBuffer*>(&buffer);

        VkBuffer vkBuffer = vulkanBuffer.GetVkBuffer();
        VmaAllocation allocation = vulkanBuffer.GetVmaAllocation();
        m_Context.Destroy([vkBuffer, allocation, allocator = &m_Allocator]() mutable
        {
            allocator->DestroyBuffer(vkBuffer, allocation);
        });
    }

    void VulkanDevice::DestroyFramebuffer(Framebuffer& framebuffer) const
    {
        VulkanFramebuffer& vulkanFramebuffer = *api_cast<VulkanFramebuffer*>(&framebuffer);

        VkDevice device = m_Context.GetVulkanLogicalDevice().GetVkDevice();
        VkFramebuffer vkFramebuffer = vulkanFramebuffer.GetVkFramebuffer();
        m_Context.Destroy([device, vkFramebuffer]() mutable
        {
            vkDestroyFramebuffer(device, vkFramebuffer, VulkanAllocator::GetCallbacks());
        });
    }

    void VulkanDevice::DestroyRenderpass(Renderpass& renderpass) const
    {
        VulkanRenderpass& vulkanRenderpass = *api_cast<VulkanRenderpass*>(&renderpass);

        VkDevice device = m_Context.GetVulkanLogicalDevice().GetVkDevice();
        VkRenderPass vkRenderpass = vulkanRenderpass.GetVkRenderPass();

        for (auto& framebuffer : vulkanRenderpass.GetFramebuffers())
            DestroyFramebuffer(framebuffer);

        m_Context.Destroy([device, vkRenderpass]() mutable
        {
            vkDestroyRenderPass(device, vkRenderpass, VulkanAllocator::GetCallbacks());
        });
    }

    void VulkanDevice::DestroyShader(Shader& shader) const
    {
        VulkanShader& vulkanShader = *api_cast<VulkanShader*>(&shader);

        VkDevice device = m_Context.GetVulkanLogicalDevice().GetVkDevice();
        VkShaderModule vkShader = vulkanShader.GetVkShaderModule();
        m_Context.Destroy([device, vkShader]() mutable
        {
            vkDestroyShaderModule(device, vkShader, VulkanAllocator::GetCallbacks());
        });
    }

    void VulkanDevice::DestroyInputLayout(InputLayout& layout) const
    {
        (void)layout;
    }

    void VulkanDevice::DestroyBindingLayout(BindingLayout& layout) const
    {
        VulkanBindingLayout& vulkanPool = *api_cast<VulkanBindingLayout*>(&layout);

        VkDevice device = m_Context.GetVulkanLogicalDevice().GetVkDevice();
        VkDescriptorSetLayout vkLayout = vulkanPool.GetVkDescriptorSetLayout();
        m_Context.Destroy([device, vkLayout]() mutable
        {
            vkDestroyDescriptorSetLayout(device, vkLayout, VulkanAllocator::GetCallbacks());
        });
    }

    void VulkanDevice::FreeBindingSetPool(BindingSetPool& pool) const
    {
        VulkanBindingSetPool& vulkanPool = *api_cast<VulkanBindingSetPool*>(&pool);

        VkDevice device = m_Context.GetVulkanLogicalDevice().GetVkDevice();
        VkDescriptorPool vkPool = vulkanPool.GetVkDescriptorPool();
        m_Context.Destroy([device, vkPool]() mutable
        {
            vkDestroyDescriptorPool(device, vkPool, VulkanAllocator::GetCallbacks());
        });
    }

    void VulkanDevice::DestroyGraphicsPipeline(GraphicsPipeline& pipeline) const
    {
        VulkanGraphicsPipeline& vulkanGraphicsPipeline = *api_cast<VulkanGraphicsPipeline*>(&pipeline);

        VkDevice device = m_Context.GetVulkanLogicalDevice().GetVkDevice();
        VkPipelineLayout vkPipelineLayout = vulkanGraphicsPipeline.GetVkPipelineLayout();
        VkPipeline vkPipeline = vulkanGraphicsPipeline.GetVkPipeline();
        m_Context.Destroy([device, vkPipelineLayout, vkPipeline]() mutable
        {
            vkDestroyPipeline(device, vkPipeline, VulkanAllocator::GetCallbacks());
            vkDestroyPipelineLayout(device, vkPipelineLayout, VulkanAllocator::GetCallbacks());
        });
    }

    void VulkanDevice::DestroyComputePipeline(ComputePipeline& pipeline) const
    {
        VulkanComputePipeline& vulkanComputePipeline = *api_cast<VulkanComputePipeline*>(&pipeline);

        VkDevice device = m_Context.GetVulkanLogicalDevice().GetVkDevice();
        VkPipelineLayout vkPipelineLayout = vulkanComputePipeline.GetVkPipelineLayout();
        VkPipeline vkPipeline = vulkanComputePipeline.GetVkPipeline();
        m_Context.Destroy([device, vkPipelineLayout, vkPipeline]() mutable
        {
            vkDestroyPipeline(device, vkPipeline, VulkanAllocator::GetCallbacks());
            vkDestroyPipelineLayout(device, vkPipelineLayout, VulkanAllocator::GetCallbacks());
        });
    }

}