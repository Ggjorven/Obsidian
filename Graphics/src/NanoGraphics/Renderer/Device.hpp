#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/DeviceSpec.hpp"
#include "NanoGraphics/Renderer/Bindings.hpp"
#include "NanoGraphics/Renderer/Image.hpp"
#include "NanoGraphics/Renderer/Buffer.hpp"
#include "NanoGraphics/Renderer/Swapchain.hpp"
#include "NanoGraphics/Renderer/CommandList.hpp"
#include "NanoGraphics/Renderer/Renderpass.hpp"
#include "NanoGraphics/Renderer/Shader.hpp"
#include "NanoGraphics/Renderer/Pipeline.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Device.hpp"
#include "NanoGraphics/Platform/Dummy/DummyDevice.hpp"

#include <Nano/Nano.hpp>

#include <span>

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Device
    ////////////////////////////////////////////////////////////////////////////////////
    class Device
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanDevice>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12Device>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyDevice>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyDevice>
        >;
    public:
        // Constructor & Destructor
        inline Device(const DeviceSpecification& specs) { m_Impl.Construct(specs); }
        ~Device() = default;

        // Methods 
        inline void Wait() const { m_Impl->Wait(); } // Note: Makes the CPU wait on the GPU to finish all operations // Note: Should not be used frequently

        inline void StartTracking(const Image& image, ImageSubresourceSpecification subresources, ResourceState currentState = ResourceState::Unknown) { m_Impl->StartTracking(image, subresources, currentState); }
        inline void StartTracking(const StagingImage& image, ResourceState currentState = ResourceState::Unknown) { m_Impl->StartTracking(image, currentState); }
        inline void StartTracking(const Buffer& buffer, ResourceState currentState = ResourceState::Unknown) { m_Impl->StartTracking(buffer, currentState); }
        inline void StopTracking(const Image& image) { m_Impl->StopTracking(image); }
        inline void StopTracking(const StagingImage& image) { m_Impl->StopTracking(image); }
        inline void StopTracking(const Buffer& buffer) { m_Impl->StopTracking(buffer); }

        inline void MapBuffer(const Buffer& buffer, void*& memory) const { return m_Impl->MapBuffer(buffer, memory); }
        inline void UnmapBuffer(const Buffer& buffer) const { return m_Impl->UnmapBuffer(buffer); }
        inline void MapStagingImage(const StagingImage& image, void*& memory) const { return m_Impl->MapStagingImage(image, memory); }
        inline void UnmapStagingImage(const StagingImage& image) const { return m_Impl->UnmapStagingImage(image); }

        inline void WriteBuffer(const Buffer& buffer, void* memory, size_t size, size_t offset = 0) const { m_Impl->WriteBuffer(buffer, memory, size, offset); }
        inline void WriteImage(const StagingImage& image, void* memory, size_t size, size_t offset = 0) const { m_Impl->WriteImage(image, memory, size, offset); }

        // Creation/Destruction methods // Note: Copy elision (RVO/NRVO) ensures object is constructed directly in the caller's stack frame.
        inline Swapchain CreateSwapchain(const SwapchainSpecification& specs) const { return Swapchain(*this, specs); }
        inline void DestroySwapchain(Swapchain& swapchain) const { return m_Impl->DestroySwapchain(swapchain); }

        inline Image CreateImage(const ImageSpecification& specs) const { return Image(*this, specs); }
        inline void DestroyImage(Image& image) const { m_Impl->DestroyImage(image); }
        inline StagingImage CreateStagingImage(const ImageSpecification& specs, CpuAccessMode cpuAccessMode = CpuAccessMode::None) const { return StagingImage(*this, specs, cpuAccessMode); }
        inline void DestroyStagingImage(StagingImage& image) const { m_Impl->DestroyStagingImage(image); }
        inline Sampler CreateSampler(const SamplerSpecification& specs) const { return Sampler(*this, specs); }
        inline void DestroySampler(Sampler& sampler) const { m_Impl->DestroySampler(sampler); }

        inline Buffer CreateBuffer(const BufferSpecification& specs) const { return Buffer(*this, specs); }
        inline void DestroyBuffer(Buffer& buffer) { m_Impl->DestroyBuffer(buffer); }

        inline Renderpass CreateRenderpass(const RenderpassSpecification& specs) const { return Renderpass(*this, specs); }
        inline void DestroyRenderpass(Renderpass& renderpass) const { m_Impl->DestroyRenderpass(renderpass); }

        inline Shader CreateShader(const ShaderSpecification& specs) const { return Shader(*this, specs); }
        inline void DestroyShader(Shader& shader) const { m_Impl->DestroyShader(shader); }

        inline InputLayout CreateInputLayout(std::span<const VertexAttributeSpecification> attributes) const { return InputLayout(*this, attributes); }
        inline InputLayout CreateInputLayout(const std::vector<VertexAttributeSpecification>& attributes) const { return CreateInputLayout(std::span<const VertexAttributeSpecification>(attributes)); }
        inline void DestroyInputLayout(InputLayout& layout) const { m_Impl->DestroyInputLayout(layout); }

        inline BindingLayout CreateBindingLayout(const BindingLayoutSpecification& specs) const { return BindingLayout(*this, specs); }
        inline BindingLayout CreateBindingLayout(const BindlessLayoutSpecification& specs) const { return BindingLayout(*this, specs); }
        inline void DestroyBindingLayout(BindingLayout& layout) const { m_Impl->DestroyBindingLayout(layout); }

        inline BindingSetPool AllocateBindingSetPool(const BindingSetPoolSpecification& specs) const { return BindingSetPool(*this, specs); }
        inline void FreeBindingSetPool(BindingSetPool& pool) const { m_Impl->FreeBindingSetPool(pool); }

        inline GraphicsPipeline CreateGraphicsPipeline(const GraphicsPipelineSpecification& specs) const { return GraphicsPipeline(*this, specs); }
        inline void DestroyGraphicsPipeline(GraphicsPipeline& pipeline) const { m_Impl->DestroyGraphicsPipeline(pipeline); }
        inline ComputePipeline CreateComputePipeline(const ComputePipelineSpecification& specs) const { return ComputePipeline(*this, specs); }
        inline void DestroyComputePipeline(ComputePipeline& pipeline) const { m_Impl->DestroyComputePipeline(pipeline); }

    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class APICaster;
    };

}