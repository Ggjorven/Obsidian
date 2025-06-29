#include "ngpch.h"
#include "Dx12Device.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12Context.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Image.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Buffer.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Swapchain.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Pipeline.hpp"

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12Device::Dx12Device(const DeviceSpecification& specs)
        : m_Context(specs.MessageCallback, specs.DestroyCallback), m_Allocator(m_Context.GetD3D12Adapter().Get(), m_Context.GetD3D12Device()), m_Resources(*api_cast<const Device*>(this)), m_StateTracker(*api_cast<const Device*>(this))
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
        //NG_ASSERT(false, "Not implemented.");
    }

    void Dx12Device::StartTracking(const Image& image, ImageSubresourceSpecification subresources, ResourceState currentState)
    {
        NG_PROFILE("Dx12Device::StartTracking()");
        m_StateTracker.StartTracking(image, subresources, currentState);
    }

    void Dx12Device::StartTracking(const StagingImage& image, ResourceState currentState)
    {
        NG_PROFILE("Dx12Device::StartTracking()");
        const Dx12StagingImage& dxStagingImage = *api_cast<const Dx12StagingImage*>(&image);
        m_StateTracker.StartTracking(*api_cast<const Buffer*>(&dxStagingImage.GetDx12Buffer()), currentState);
    }

    void Dx12Device::StartTracking(const Buffer& buffer, ResourceState currentState)
    {
        NG_PROFILE("Dx12Device::StartTracking()");
        m_StateTracker.StartTracking(buffer, currentState);
    }

    void Dx12Device::StopTracking(const Image& image)
    {
        NG_PROFILE("Dx12Device::StopTracking()");
        m_StateTracker.StopTracking(image);
    }

    void Dx12Device::StopTracking(const StagingImage& image)
    {
        NG_PROFILE("Dx12Device::StopTracking()");
        const Dx12StagingImage& dxStagingImage = *api_cast<const Dx12StagingImage*>(&image);
        m_StateTracker.StopTracking(*api_cast<const Buffer*>(&dxStagingImage.GetDx12Buffer()));
    }

    void Dx12Device::StopTracking(const Buffer& buffer)
    {
        NG_PROFILE("Dx12Device::StopTracking()");
        m_StateTracker.StopTracking(buffer);
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
        const Dx12StagingImage& dxStagingImage = *api_cast<const Dx12StagingImage*>(&image);
    }

    void Dx12Device::UnmapStagingImage(const StagingImage& image) const
    {
        const Dx12StagingImage& dxStagingImage = *api_cast<const Dx12StagingImage*>(&image);

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
        m_Context.Destroy([resource = dxImage.GetD3D12Resource(), allocation = dxImage.GetD3D12MAAllocation()]() {}); // Note: Holding a reference to the resource is enough to keep it alive (and destroy when the scope ends)

        dxImage.m_Resource = nullptr;
        dxImage.m_Allocation = nullptr;
    }

    void Dx12Device::DestroySubresourceViews(Image& image) const
    {
        Dx12Image& dxImage = *api_cast<Dx12Image*>(&image);

        for (auto& [_, view] : dxImage.GetImageViews())
        {
            switch (view.GetUsage())
            {
            case ImageSubresourceViewUsage::SRV:
            case ImageSubresourceViewUsage::UAV:
                // Note: SRV & UAV's are not freeable because of manual indexing
                break;

            case ImageSubresourceViewUsage::RTV:
                m_Resources.GetRTVHeap().Free(view.GetIndex());
                break;

            case ImageSubresourceViewUsage::DSV:
                m_Resources.GetDSVHeap().Free(view.GetIndex());
                break;

            default:
                NG_UNREACHABLE();
                break;
            }
        }

        dxImage.GetImageViews().clear();
    }

    void Dx12Device::DestroyStagingImage(StagingImage& stagingImage) const
    {
        Dx12StagingImage& dxStagingImage = *api_cast<Dx12StagingImage*>(&stagingImage);

        DestroyBuffer(*api_cast<Buffer*>(&dxStagingImage.GetDx12Buffer()));
    }

    void Dx12Device::DestroySampler(Sampler& sampler) const
    {
        (void)sampler;
    }

    void Dx12Device::DestroyBuffer(Buffer& buffer) const
    {
        Dx12Buffer& dxBuffer = *api_cast<Dx12Buffer*>(&buffer);

        m_Context.Destroy([resource = dxBuffer.GetD3D12Resource(), allocation = dxBuffer.GetD3D12MAAllocation()]() {}); // Note: Holding a reference to the resource is enough to keep it alive (and destroy when the scope ends)

        dxBuffer.m_Resource = nullptr;
        dxBuffer.m_Allocation = nullptr;
    }

    void Dx12Device::DestroyFramebuffer(Framebuffer& framebuffer) const
    {
        (void)framebuffer;
    }

    void Dx12Device::DestroyRenderpass(Renderpass& renderpass) const
    {
        (void)renderpass;
    }

    void Dx12Device::DestroyShader(Shader& shader) const
    {
        (void)shader;
    }

    void Dx12Device::DestroyInputLayout(InputLayout& layout) const
    {
        (void)layout;
    }

    void Dx12Device::DestroyBindingLayout(BindingLayout& layout) const
    {
        (void)layout;
    }

    void Dx12Device::FreeBindingSetPool(BindingSetPool& pool) const
    {
        (void)pool;
    }

    void Dx12Device::DestroyGraphicsPipeline(GraphicsPipeline& pipeline) const
    {
        Dx12GraphicsPipeline& dxPipeline = *api_cast<Dx12GraphicsPipeline*>(&pipeline);

        m_Context.Destroy([rootSignature = dxPipeline.GetD3D12RootSignature(), pipelineState = dxPipeline.GetD3D12PipelineState()]() {}); // Note: Holding a reference to the resource is enough to keep it alive (and destroy when the scope ends)

        dxPipeline.m_RootSignature = nullptr;
        dxPipeline.m_PipelineState = nullptr;
    }

    void Dx12Device::DestroyComputePipeline(ComputePipeline& pipeline) const
    {
        // TODO: ...
    }

}