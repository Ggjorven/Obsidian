#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/DeviceSpec.hpp"

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
}

namespace Nano::Graphics::Internal
{

    class DummyDevice;

#if 1 //defined(NG_API_DUMMY)
    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanDevice
    ////////////////////////////////////////////////////////////////////////////////////
    class DummyDevice : public Traits::NoMove
    {
    public:
        // Constructors & Destructor
        inline constexpr DummyDevice(const DeviceSpecification& specs) { (void)specs; }
        constexpr ~DummyDevice() = default;

        // Methods
        inline constexpr void Wait() const {}

        inline constexpr void MapBuffer(const Buffer& buffer, void*& memory) const { (void)buffer; memory = nullptr; }
        inline constexpr void UnmapBuffer(const Buffer& buffer) const { (void)buffer; }
        inline constexpr void MapStagingImage(const StagingImage& image, void*& memory) const { (void)image; memory = nullptr; }
        inline constexpr void UnmapStagingImage(const StagingImage& image) const { (void)image; }

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
    };
#endif

}