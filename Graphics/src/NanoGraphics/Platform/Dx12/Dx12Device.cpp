#include "ngpch.h"
#include "Dx12Device.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12Context.hpp"

#include <type_traits>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace Nano::Graphics::Internal
{

    namespace
    {

        ////////////////////////////////////////////////////////////////////////////////////
        // Conversion functions
        ////////////////////////////////////////////////////////////////////////////////////
        static constexpr auto GetDx12FeatureLevel() -> decltype(std::to_underlying(D3D_FEATURE_LEVEL_12_1))
        {
            NANO_ASSERT((std::get<0>(Dx12Context::Version) == 12), "[Dx12Device] Dx12Version must start with 12.");
            
            switch (std::get<1>(Dx12Context::Version))
            {
            case 0:         return D3D_FEATURE_LEVEL_12_0;
            case 1:         return D3D_FEATURE_LEVEL_12_1;
            case 2:         return D3D_FEATURE_LEVEL_12_2;

            default:
                break;
            }

            return D3D_FEATURE_LEVEL_12_0;
        }

    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12Device::Dx12Device(const DeviceSpecification& specs)
        : m_Context(specs.MessageCallback, specs.DestroyCallback)
    {
        DX_VERIFY(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_Device)));
        m_Context.InitMessageQueue(m_Device);
    }

    Dx12Device::~Dx12Device()
    {
        m_Context.Destroy([device = m_Device]()
        {
            device->Release();
        });
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12Device::Wait() const
    {
    }

    void Dx12Device::StartTracking(const Image& image, ImageSubresourceSpecification subresources, ResourceState currentState)
    {
        NG_PROFILE("Dx12Device::StartTracking()");
    }

    void Dx12Device::StartTracking(const StagingImage& image, ResourceState currentState)
    {
        NG_PROFILE("Dx12Device::StartTracking()");
    }

    void Dx12Device::StartTracking(const Buffer& buffer, ResourceState currentState)
    {
        NG_PROFILE("Dx12Device::StartTracking()");
    }

    void Dx12Device::StopTracking(const Image& image)
    {
        NG_PROFILE("Dx12Device::StopTracking()");
    }

    void Dx12Device::StopTracking(const StagingImage& image)
    {
        NG_PROFILE("Dx12Device::StopTracking()");
    }

    void Dx12Device::StopTracking(const Buffer& buffer)
    {
        NG_PROFILE("Dx12Device::StopTracking()");
    }

    void Dx12Device::MapBuffer(const Buffer& buffer, void*& memory) const
    {
        memory = nullptr;
    }

    void Dx12Device::UnmapBuffer(const Buffer& buffer) const
    {
    }

    void Dx12Device::MapStagingImage(const StagingImage& image, void*& memory) const
    {
        memory = nullptr;
    }

    void Dx12Device::UnmapStagingImage(const StagingImage& image) const
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Destruction methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12Device::DestroySwapchain(Swapchain& swapchain) const
    {
    }

    void Dx12Device::DestroyImage(Image& image) const
    {
    }

    void Dx12Device::DestroySubresourceViews(Image& image) const
    {
    }

    void Dx12Device::DestroyStagingImage(StagingImage& stagingImage) const
    {
    }

    void Dx12Device::DestroySampler(Sampler& sampler) const
    {
    }

    void Dx12Device::DestroyBuffer(Buffer& buffer) const
    {
    }

    void Dx12Device::DestroyFramebuffer(Framebuffer& framebuffer) const
    {
    }

    void Dx12Device::DestroyRenderpass(Renderpass& renderpass) const
    {
    }

    void Dx12Device::DestroyShader(Shader& shader) const
    {
    }

    void Dx12Device::DestroyInputLayout(InputLayout& layout) const
    {
    }

    void Dx12Device::DestroyBindingLayout(BindingLayout& layout) const
    {
    }

    void Dx12Device::FreeBindingSetPool(BindingSetPool& pool) const
    {
    }

    void Dx12Device::DestroyGraphicsPipeline(GraphicsPipeline& pipeline) const
    {
    }

    void Dx12Device::DestroyComputePipeline(ComputePipeline& pipeline) const
    {
    }

}