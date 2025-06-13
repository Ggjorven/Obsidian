#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/DeviceSpec.hpp"
#include "NanoGraphics/Renderer/Image.hpp"
#include "NanoGraphics/Renderer/Buffer.hpp"
#include "NanoGraphics/Renderer/Swapchain.hpp"
#include "NanoGraphics/Renderer/CommandList.hpp"
#include "NanoGraphics/Renderer/Renderpass.hpp"
#include "NanoGraphics/Renderer/Shader.hpp"
#include "NanoGraphics/Renderer/Pipeline.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Device
    ////////////////////////////////////////////////////////////////////////////////////
    class Device
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanDevice>
        >;
    public:
        // Constructor & Destructor
        inline Device(const DeviceSpecification& specs)
            : m_Device(specs) {}
        ~Device() = default;

        // Methods 
        inline void Wait() const { m_Device.Wait(); } // Note: Makes the CPU wait on the GPU to finish all operations

        // Creation/Destruction methods // Note: Copy elision (RVO/NRVO) ensures object is constructed directly in the caller's stack frame.
        inline Swapchain CreateSwapchain(const SwapchainSpecification& specs) const { return Swapchain(*this, specs); }
        inline void DestroySwapchain(Swapchain& swapchain) const { return m_Device.DestroySwapchain(swapchain); }

        inline Image CreateImage(const ImageSpecification& specs) const { return Image(*this, specs); }
        inline void DestroyImage(Image& image) const { m_Device.DestroyImage(image); }
        inline StagingImage CreateStagingImage(const ImageSpecification& specs, CpuAccessMode cpuAccessMode = CpuAccessMode::None) const { return StagingImage(*this, specs, cpuAccessMode); }
        inline void DestroyStagingImage(StagingImage& image) const { m_Device.DestroyStagingImage(image); }
        inline Sampler CreateSampler(const SamplerSpecification& specs) const { return Sampler(*this, specs); }
        inline void DestroySampler(Sampler& sampler) const { m_Device.DestroySampler(sampler); }

        inline Buffer CreateBuffer(const BufferSpecification& specs) const { return Buffer(*this, specs); }
        inline void DestroyBuffer(Buffer& buffer) { m_Device.DestroyBuffer(buffer); }

        inline Renderpass CreateRenderpass(const RenderpassSpecification& specs) const { return Renderpass(*this, specs); }
        inline void DestroyRenderpass(Renderpass& renderpass) const { m_Device.DestroyRenderpass(renderpass); }

        inline Shader CreateShader(const ShaderSpecification& specs) const { return Shader(*this, specs); }
        inline void DestroyShader(Shader& shader) const { m_Device.DestroyShader(shader); }

        inline GraphicsPipeline CreateGraphicsPipeline(const GraphicsPipelineSpecification& specs) const { return GraphicsPipeline(*this, specs); }
        inline void DestroyGraphicsPipeline(GraphicsPipeline& pipeline) const { m_Device.DestroyGraphicsPipeline(pipeline); }

    private:
        Type m_Device;
    };

}