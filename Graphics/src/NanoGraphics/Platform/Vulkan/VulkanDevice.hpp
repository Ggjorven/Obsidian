#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/DeviceSpec.hpp"
#include "NanoGraphics/Renderer/StateTracker.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanContext.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{
    class Swapchain;
    class Image;
    class StagingImage;
    class Sampler;
    class InputLayout;
    class BindingLayout;
    class BindingSetPool;
    class Buffer;
    class Framebuffer;
    class Renderpass;
    class Shader;
    class GraphicsPipeline;
    class ComputePipeline;
}

namespace Nano::Graphics::Internal
{

    class VulkanDevice;

#if defined(NG_API_VULKAN)
    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanDevice
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanDevice
    {
    public:
        // Constructors & Destructor
        VulkanDevice(const DeviceSpecification& specs);
        ~VulkanDevice();

        // Methods
        void Wait() const;

        void StartTracking(const Image& image, ImageSubresourceSpecification subresources, ResourceState currentState);
        void StartTracking(const StagingImage& image, ResourceState currentState);
        void StartTracking(const Buffer& buffer, ResourceState currentState);
        void StopTracking(const Image& image);
        void StopTracking(const StagingImage& image);
        void StopTracking(const Buffer& buffer);

        void MapBuffer(const Buffer& buffer, void*& memory) const;
        void UnmapBuffer(const Buffer& buffer) const;

        void WriteBuffer(const Buffer& buffer, const void* memory, size_t size, size_t srcOffset, size_t dstOffset) const;
        void WriteImage(const StagingImage& image, const ImageSliceSpecification& slice, const void* memory, size_t size) const;

        // Destruction methods
        void DestroySwapchain(Swapchain& swapchain) const;

        void DestroyImage(Image& image) const;
        void DestroySubresourceViews(Image& image) const;
        void DestroyStagingImage(StagingImage& stagingImage) const;
        void DestroySampler(Sampler& sampler) const;

        void DestroyBuffer(Buffer& buffer) const;

        void DestroyFramebuffer(Framebuffer& framebuffer) const;
        void DestroyRenderpass(Renderpass& renderpass) const;

        void DestroyShader(Shader& shader) const;

        void DestroyInputLayout(InputLayout& layout) const;
        void DestroyBindingLayout(BindingLayout& layout) const;

        void FreeBindingSetPool(BindingSetPool& pool) const;

        void DestroyGraphicsPipeline(GraphicsPipeline& pipeline) const;
        void DestroyComputePipeline(ComputePipeline& pipeline) const;

        // Internal Getters
        inline const VulkanContext& GetContext() const { return m_Context; }
        inline const VulkanAllocator& GetAllocator() const { return m_Allocator; }
        inline const StateTracker& GetTracker() const { return m_StateTracker; }

    private:
        VulkanContext m_Context;
        VulkanAllocator m_Allocator;
        mutable StateTracker m_StateTracker;
    };
#endif

}