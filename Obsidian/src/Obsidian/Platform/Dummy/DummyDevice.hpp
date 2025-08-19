#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/DeviceSpec.hpp"

#include <Nano/Nano.hpp>

namespace Obsidian
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
}

namespace Obsidian::Internal
{

    class DummyDevice;

#if 1 //defined(OB_API_DUMMY)
    ////////////////////////////////////////////////////////////////////////////////////
    // DummyDevice
    ////////////////////////////////////////////////////////////////////////////////////
    class DummyDevice
    {
    public:
        // Constructors & Destructor
        inline constexpr DummyDevice(const DeviceSpecification& specs) { (void)specs; }
        constexpr ~DummyDevice() = default;

        // Methods
        inline constexpr void Wait() const {}

        inline constexpr void MapBuffer(const Buffer& buffer, void*& memory) const { (void)buffer; memory = nullptr; }
        inline constexpr void UnmapBuffer(const Buffer& buffer) const { (void)buffer; }

        inline constexpr void WriteBuffer(const Buffer& buffer, const void* memory, size_t size, size_t srcOffset, size_t dstOffset) const { (void)buffer; (void)memory; (void)size; (void)srcOffset; (void)dstOffset; }
        inline constexpr void WriteImage(const StagingImage& image, const ImageSliceSpecification& slice, const void* memory, size_t size) const { (void)image; (void)slice; (void)memory; (void)size; }

        inline constexpr void StartTracking(const Image& image, ImageSubresourceSpecification subresources, ResourceState currentState) { (void)image; (void)subresources; (void)currentState; }
        inline constexpr void StartTracking(const StagingImage& image, ResourceState currentState) { (void)image; (void)currentState; }
        inline constexpr void StartTracking(const Buffer& buffer, ResourceState currentState) { (void)buffer; (void)currentState; }
        inline constexpr void StopTracking(const Image& image) { (void)image; }
        inline constexpr void StopTracking(const StagingImage& image) { (void)image; }
        inline constexpr void StopTracking(const Buffer& buffer) { (void)buffer; }

        // Destruction methods
        inline constexpr void DestroySwapchain(Swapchain& swapchain) const { (void)swapchain; }

        inline constexpr void DestroyImage(Image& image) const { (void)image; }
        inline constexpr void DestroySubresourceViews(Image& image) const { (void)image; }
        inline constexpr void DestroyStagingImage(StagingImage& stagingImage) const { (void)stagingImage; }
        inline constexpr void DestroySampler(Sampler& sampler) const { (void)sampler; }

        inline constexpr void DestroyBuffer(Buffer& buffer) const { (void)buffer; }

        inline constexpr void DestroyFramebuffer(Framebuffer& framebuffer) const { (void)framebuffer; }
        inline constexpr void DestroyRenderpass(Renderpass& renderpass) const { (void)renderpass; }

        inline constexpr void DestroyShader(Shader& shader) const { (void)shader; }

        inline constexpr void DestroyInputLayout(InputLayout& layout) const { (void)layout; }
        inline constexpr void DestroyBindingLayout(BindingLayout& layout) const { (void)layout; }

        inline constexpr void FreeBindingSetPool(BindingSetPool& pool) const { (void)pool; }

        inline constexpr void DestroyGraphicsPipeline(GraphicsPipeline& pipeline) const { (void)pipeline; }
        inline constexpr void DestroyComputePipeline(ComputePipeline& pipeline) const { (void)pipeline; }
    };
#endif

}