#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/DeviceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"
#include "NanoGraphics/Renderer/ResourceSpec.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Context.hpp"

#include <Nano/Nano.hpp>

#include <tuple>

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

    class Dx12Device;

#if defined(NG_API_DX12)
    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12Device
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12Device
    {
    public:
        inline constexpr static const std::tuple<uint8_t, uint8_t> Version = { 12, 1 };
    public:
        // Constructors & Destructor
        Dx12Device(const DeviceSpecification& specs);
        ~Dx12Device();

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
        void MapStagingImage(const StagingImage& image, void*& memory) const;
        void UnmapStagingImage(const StagingImage& image) const;

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

        // Internal getters
        inline const Dx12Context& GetContext() const { return m_Context; }

    private:
        Dx12Context m_Context;
    };
#endif

}