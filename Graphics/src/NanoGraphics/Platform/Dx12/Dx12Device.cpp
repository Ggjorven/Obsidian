#include "ngpch.h"
#include "Dx12Device.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12Context.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Image.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Swapchain.hpp"

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12Device::Dx12Device(const DeviceSpecification& specs)
        : m_Context(specs.MessageCallback, specs.DestroyCallback), m_Resources(*api_cast<const Device*>(this))
    {
    }

    Dx12Device::~Dx12Device()
    {
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
        Dx12Swapchain& dxSwapchain = *api_cast<Dx12Swapchain*>(&swapchain);

        std::array<HANDLE, Information::FramesInFlight> events;
        const auto& valuesAndEvents = dxSwapchain.GetValuesAndEvents();
        for (size_t i = 0; i < valuesAndEvents.size(); i++)
            events[i] = valuesAndEvents[i].second;

        for (uint8_t i = 0; i < dxSwapchain.GetImageCount(); i++)
            DestroyImage(dxSwapchain.GetImage(i));

        m_Context.Destroy([swapchain = dxSwapchain.GetDXGISwapChain(), fence = dxSwapchain.GetD3D12Fence(), events = std::move(events)]() // Note: Holding a reference to the resource is enough to keep it alive (and destroy when the scope ends)
        {
            for (const auto& event : events)
                CloseHandle(event);
        });

        dxSwapchain.m_Swapchain = nullptr;
        dxSwapchain.m_Fence = nullptr;
    }

    void Dx12Device::DestroyImage(Image& image) const
    {
        Dx12Image& dxImage = *api_cast<Dx12Image*>(&image);

        DestroySubresourceViews(image);
        m_Context.Destroy([resource = dxImage.GetD3D12Resource()]() {}); // Note: Holding a reference to the resource is enough to keep it alive (and destroy when the scope ends)

        dxImage.m_Resource = nullptr;
    }

    void Dx12Device::DestroySubresourceViews(Image& image) const
    {
        Dx12Image& dxImage = *api_cast<Dx12Image*>(&image);

        for (auto& [_, view] : dxImage.GetImageViews())
        {
            m_Resources.GetRTVHeap().Free(view.GetHandle());
            // TODO: Somehow know what type a view is and free them
        }

        dxImage.GetImageViews().clear();
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