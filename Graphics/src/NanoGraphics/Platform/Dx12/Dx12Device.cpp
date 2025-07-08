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
        NG_PROFILE("Dx12Device::Wait()");

        uint64_t fenceValue = 1;

        Nano::Memory::StaticVector<HANDLE, static_cast<size_t>(CommandQueue::Count)> events;
        std::unordered_set<ID3D12CommandQueue*> queues;
        queues.reserve(events.size());

        // Retrieve all queues
        for (const auto& queue : m_Context.GetD3D12CommandQueues())
            queues.insert(queue.Get());
        // Create an event for all unique queues
        for (size_t i = 0; i < queues.size(); i++)
            events.emplace_back() = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        DxPtr<ID3D12Fence> fence; // Note: Will be destroyed at end of the scope
        m_Context.GetD3D12Device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

        // Signal all unique queues
        for (auto& queue : queues)
            queue->Signal(fence.Get(), fenceValue);
        // Create the event wait
        for (auto& event : events)
            fence->SetEventOnCompletion(fenceValue, event);

        // Wait for all events and destroy
        for (auto& event : events)
        {
            WaitForSingleObject(event, INFINITE);
            CloseHandle(event);
        }
    }

    void Dx12Device::StartTracking(const Image& image, ImageSubresourceSpecification subresources, ResourceState currentState)
    {
        NG_PROFILE("Dx12Device::StartTracking()");

        // Note: On Dx12 we create the resource already initialized with the PermanentState, so check
        // if it was set if so, when currentState == ResourceState::Unknown it's actually the PermanentState.
        m_StateTracker.StartTracking(image, subresources, (image.GetSpecification().HasPermanentState() ? image.GetSpecification().PermanentState : currentState));
    }

    void Dx12Device::StartTracking(const StagingImage& image, ResourceState currentState)
    {
        NG_PROFILE("Dx12Device::StartTracking()");
        const Dx12StagingImage& dxStagingImage = *api_cast<const Dx12StagingImage*>(&image);

        // Note: On Dx12 we create the resource already initialized with the PermanentState, so check
        // if it was set if so, when currentState == ResourceState::Unknown it's actually the PermanentState.
        m_StateTracker.StartTracking(*api_cast<const Buffer*>(&dxStagingImage.GetDx12Buffer()), (image.GetSpecification().HasPermanentState() ? image.GetSpecification().PermanentState : currentState));
    }

    void Dx12Device::StartTracking(const Buffer& buffer, ResourceState currentState)
    {
        NG_PROFILE("Dx12Device::StartTracking()");

        // Note: On Dx12 we create the resource already initialized with the PermanentState, so check
        // if it was set if so, when currentState == ResourceState::Unknown it's actually the PermanentState.
        m_StateTracker.StartTracking(buffer, (buffer.GetSpecification().HasPermanentState() ? buffer.GetSpecification().PermanentState : currentState));
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
        NG_PROFILE("Dx12Device::MapBuffer()");
        const Dx12Buffer& dxBuffer = *api_cast<const Dx12Buffer*>(&buffer);
        dxBuffer.GetD3D12Resource()->Map(0, nullptr, &memory);
    }

    void Dx12Device::UnmapBuffer(const Buffer& buffer) const
    {
        NG_PROFILE("Dx12Device::UnmapBuffer()");
        const Dx12Buffer& dxBuffer = *api_cast<const Dx12Buffer*>(&buffer);
        dxBuffer.GetD3D12Resource()->Unmap(0, nullptr);
    }

    void Dx12Device::WriteBuffer(const Buffer& buffer, const void* memory, size_t size, size_t srcOffset, size_t dstOffset) const
    {
        NG_PROFILE("Dx12Device::WriteBuffer()");

        NG_ASSERT((size + dstOffset <= buffer.GetSpecification().Size), "[VkDevice] Size + offset exceeds buffer size.");

        void* bufferMemory;
        MapBuffer(buffer, bufferMemory);

        std::memcpy(static_cast<uint8_t*>(bufferMemory) + dstOffset, static_cast<const uint8_t*>(memory) + srcOffset, size);

        UnmapBuffer(buffer);
    }

    void Dx12Device::WriteImage(const StagingImage& image, const ImageSliceSpecification& slice, const void* memory, size_t size) const
    {
        NG_PROFILE("Dx12Device::WriteImage()");

        const Dx12StagingImage& dxImage = *api_cast<const Dx12StagingImage*>(&image);
        const Dx12Buffer& dxBuffer = dxImage.GetDx12Buffer();
        const Buffer& buffer = *api_cast<const Buffer*>(&dxBuffer);

        const FormatInfo& formatInfo = FormatToFormatInfo(dxImage.GetSpecification().ImageFormat);
        ImageSliceSpecification resSlice = ResolveImageSlice(slice, image.GetSpecification());

        void* imageMemory = nullptr;
        MapBuffer(buffer, imageMemory);

        Dx12StagingImage::SliceRegion region = dxImage.GetSliceRegion(resSlice.ImageMipLevel, resSlice.ImageArraySlice);

        // Dimensions
        UINT width = region.Footprint.Footprint.Width;
        UINT height = region.Footprint.Footprint.Height;
        UINT depth = region.Footprint.Footprint.Depth;
        UINT rowPitch = region.Footprint.Footprint.RowPitch;

        // Block-compressed and uncompressed support
        UINT blocksWide = (width + formatInfo.BlockSize - 1) / formatInfo.BlockSize;
        UINT blocksHigh = (height + formatInfo.BlockSize - 1) / formatInfo.BlockSize;

        UINT rowSize = blocksWide * formatInfo.BytesPerBlock;
        UINT slicePitch = rowPitch * blocksHigh;

        NG_ASSERT((size <= static_cast<size_t>(slicePitch) * depth), "[Dx12Device] Source memory too small.");

        uint8_t* dstBase = static_cast<uint8_t*>(imageMemory) + region.Offset;
        const uint8_t* srcBase = static_cast<const uint8_t*>(memory);

        // Note: We have to manually do every row, since dx12
        // doesn't tightly pack the buffer like vulkan.
        size_t srcOffset = 0;
        for (UINT z = 0; z < depth; z++)
        {
            uint8_t* dstSlice = dstBase + z * slicePitch;

            for (UINT y = 0; y < blocksHigh; y++)
            {
                void* dst = dstSlice + y * rowPitch;
                const void* src = srcBase + srcOffset;

                std::memcpy(dst, src, rowSize);
                srcOffset += rowSize; // Note: Tightly packed source
            }
        }

        UnmapBuffer(buffer);
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
        Dx12ComputePipeline& dxPipeline = *api_cast<Dx12ComputePipeline*>(&pipeline);

        m_Context.Destroy([rootSignature = dxPipeline.GetD3D12RootSignature(), pipelineState = dxPipeline.GetD3D12PipelineState()]() {}); // Note: Holding a reference to the resource is enough to keep it alive (and destroy when the scope ends)

        dxPipeline.m_RootSignature = nullptr;
        dxPipeline.m_PipelineState = nullptr;
    }

}