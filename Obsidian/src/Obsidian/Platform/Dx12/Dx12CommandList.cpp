#include "obpch.h"
#include "Dx12CommandList.hpp"

#include "Obsidian/Core/Logging.hpp"
#include "Obsidian/Core/Information.hpp"
#include "Obsidian/Core/Window.hpp"
#include "Obsidian/Utils/Profiler.hpp"

#include "Obsidian/Renderer/CommandList.hpp"
#include "Obsidian/Renderer/Swapchain.hpp"
#include "Obsidian/Renderer/Renderpass.hpp"
#include "Obsidian/Renderer/Framebuffer.hpp"
#include "Obsidian/Renderer/Buffer.hpp"
#include "Obsidian/Renderer/Image.hpp"
#include "Obsidian/Renderer/Pipeline.hpp"
#include "Obsidian/Renderer/Bindings.hpp"

#include "Obsidian/Platform/Dx12/Dx12Device.hpp"
#include "Obsidian/Platform/Dx12/Dx12Buffer.hpp"
#include "Obsidian/Platform/Dx12/Dx12Image.hpp"
#include "Obsidian/Platform/Dx12/Dx12Resources.hpp"
#include "Obsidian/Platform/Dx12/Dx12Renderpass.hpp"
#include "Obsidian/Platform/Dx12/Dx12Framebuffer.hpp"
#include "Obsidian/Platform/Dx12/Dx12Pipeline.hpp"
#include "Obsidian/Platform/Dx12/Dx12Bindings.hpp"

namespace Obsidian::Internal
{

    namespace
    {

        ////////////////////////////////////////////////////////////////////////////////////
        // Helper method
        ////////////////////////////////////////////////////////////////////////////////////
        D3D12_COMMAND_LIST_TYPE CommandQueueToD3D12CommandListType(CommandQueue queue)
        {
            switch (queue)
            {
            case CommandQueue::Graphics:
            case CommandQueue::Present:
                return D3D12_COMMAND_LIST_TYPE_DIRECT;

            case CommandQueue::Compute:
                return D3D12_COMMAND_LIST_TYPE_COMPUTE;

            default:
                OB_UNREACHABLE();
                break;
            }

            return D3D12_COMMAND_LIST_TYPE_NONE;
        }

    }

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
	Dx12CommandListPool::Dx12CommandListPool(Swapchain& swapchain, const CommandListPoolSpecification& specs)
		: m_Swapchain(*api_cast<Dx12Swapchain*>(&swapchain)), m_Specification(specs)
	{
		DX_VERIFY(m_Swapchain.GetDx12Device().GetContext().GetD3D12Device()->CreateCommandAllocator(CommandQueueToD3D12CommandListType(specs.Queue), IID_PPV_ARGS(&m_CommandAllocator)));

        if constexpr (Information::Validation)
        {
            if (!m_Specification.DebugName.empty())
                m_Swapchain.GetDx12Device().GetContext().SetDebugName(m_CommandAllocator.Get(), std::string(m_Specification.DebugName));
        }
    }

	Dx12CommandListPool::~Dx12CommandListPool()
	{
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void Dx12CommandListPool::FreeList(CommandList& list) const
	{
        Dx12CommandList& dxCommandList = *api_cast<Dx12CommandList*>(&list);

        m_Swapchain.GetDx12Device().GetContext().Destroy([commandList = dxCommandList.GetID3D12GraphicsCommandList(), idleEvent = dxCommandList.GetWaitIdleEvent()]() { CloseHandle(idleEvent); }); // Note: Holding a reference to the resource is enough to keep it alive (and destroy when the scope ends)

        dxCommandList.m_CommandList = nullptr;
	}

	void Dx12CommandListPool::FreeLists(std::span<CommandList*> lists) const
	{
        for (auto list : lists)
            FreeList(*list);
	}

	void Dx12CommandListPool::Reset() const
	{
		m_CommandAllocator->Reset();
	}

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12CommandList::Dx12CommandList(CommandListPool& pool, const CommandListSpecification& specs)
        : m_Pool(*api_cast<Dx12CommandListPool*>(&pool)), m_Specification(specs)
    {
        DxPtr<ID3D12CommandList> list;
        DX_VERIFY(m_Pool.GetDx12Swapchain().GetDx12Device().GetContext().GetD3D12Device()->CreateCommandList(0, CommandQueueToD3D12CommandListType(m_Pool.GetSpecification().Queue), m_Pool.GetD3D12CommandAllocator().Get(), nullptr, IID_PPV_ARGS(&list)));
        
        DX_VERIFY(list->QueryInterface(IID_PPV_ARGS(&m_CommandList)));

        DX_VERIFY(m_CommandList->Close());

        m_WaitIdleEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

        if constexpr (Information::Validation)
        {
            if (!m_Specification.DebugName.empty())
                m_Pool.GetDx12Swapchain().GetDx12Device().GetContext().SetDebugName(m_CommandList.Get(), std::string(m_Specification.DebugName));
        }
    }

    Dx12CommandList::~Dx12CommandList()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12CommandList::Open()
    {
        OB_PROFILE("Dx12CommandList::Open()");
        DX_VERIFY(m_CommandList->Reset(m_Pool.GetD3D12CommandAllocator().Get(), nullptr));
    }

    void Dx12CommandList::Close()
    {
        OB_PROFILE("Dx12CommandList::Close()");
        DX_VERIFY(m_CommandList->Close());

        m_CurrentGraphicsPipeline = nullptr;
    }

    void Dx12CommandList::Submit(const CommandListSubmitArgs& args)
    {
        OB_PROFILE("VulkanCommandBuffer::Submit()");

        std::span<const CommandList*> waitOn;
        std::visit([&](auto&& arg)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::vector<const CommandList*>>)
                waitOn = const_cast<std::vector<const CommandList*>&>(arg); // Note: This is worst thing I have done in my life. I can never recover.
            else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::span<const CommandList*>>)
                waitOn = arg;
        }, args.WaitOnLists);

        auto queue = m_Pool.GetDx12Swapchain().GetDx12Device().GetContext().GetD3D12CommandQueue(m_Pool.GetSpecification().Queue);
        for (auto list : waitOn)
        {
            DX_VERIFY(queue->Wait(m_Pool.GetDx12Swapchain().GetD3D12Fence().Get(), m_Pool.GetDx12Swapchain().GetPreviousCommandListWaitValue(*api_cast<const Dx12CommandList*>(list))));
        }

        // Note: Waiting on swapchain image is not a thing that needs to be handled manually for DX12
        
        ID3D12CommandList* lists[] = { m_CommandList.Get() };
        queue->ExecuteCommandLists(1, lists);

        m_SignaledValue = m_Pool.GetDx12Swapchain().RetrieveCommandListWaitValue(*this);
        DX_VERIFY(queue->Signal(m_Pool.GetDx12Swapchain().GetD3D12Fence().Get(), m_SignaledValue));
        
        if (args.OnFinishMakeSwapchainPresentable)
            m_Pool.GetDx12Swapchain().SetPresentableValue(m_SignaledValue);
    }

    void Dx12CommandList::WaitTillComplete() const
    {
        OB_PROFILE("Dx12CommandList::WaitTillComplete()");

        DxPtr<ID3D12CommandQueue> queue = m_Pool.GetDx12Swapchain().GetDx12Device().GetContext().GetD3D12CommandQueue(m_Pool.GetSpecification().Queue);
        DxPtr<ID3D12Fence> fence = m_Pool.GetDx12Swapchain().GetD3D12Fence();

        fence->SetEventOnCompletion(m_SignaledValue, m_WaitIdleEvent);
        WaitForSingleObject(m_WaitIdleEvent, INFINITE);
    }

    void Dx12CommandList::CommitBarriers()
    {
        OB_PROFILE("Dx12CommandList::CommitBarriers()");

        auto& imageBarriers = m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().GetImageBarriers(*api_cast<const CommandList*>(this));
        auto& bufferBarriers = m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().GetBufferBarriers(*api_cast<const CommandList*>(this));

        if (imageBarriers.empty() && bufferBarriers.empty())
            return;

        std::vector<D3D12_RESOURCE_BARRIER> resourceBarriers;
        resourceBarriers.reserve(imageBarriers.size() + bufferBarriers.size());

        // Image barriers
        for (const auto& imageBarrier : imageBarriers)
        {
            Dx12Image& dxImage = *api_cast<Dx12Image*>(imageBarrier.ImagePtr);

            D3D12_RESOURCE_BARRIER barrier = {};
            D3D12_RESOURCE_STATES stateBefore = ResourceStateToD3D12ResourceStates(imageBarrier.StateBefore);
            D3D12_RESOURCE_STATES stateAfter = ResourceStateToD3D12ResourceStates(imageBarrier.StateAfter);
            
            if (stateBefore != stateAfter)
            {
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Transition.StateBefore = stateBefore;
                barrier.Transition.StateAfter = stateAfter;
                barrier.Transition.pResource = dxImage.GetD3D12Resource().Get();
                
                if (imageBarrier.EntireTexture)
                {
                    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                    resourceBarriers.push_back(barrier);
                }
                else
                {
                    for (uint8_t plane = 0; plane < dxImage.GetPlaneCount(); plane++)
                    {
                        barrier.Transition.Subresource = CalculateSubresource(imageBarrier.ImageMipLevel, imageBarrier.ImageArraySlice, plane, dxImage.GetSpecification().MipLevels, dxImage.GetSpecification().ArraySize);
                        resourceBarriers.push_back(barrier);
                    }
                }
            }
            else if (stateAfter & D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
            {
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
                barrier.UAV.pResource = dxImage.GetD3D12Resource().Get();
                resourceBarriers.push_back(barrier);
            }
        }

        // Buffer barriers
        for (const auto& bufferBarrier : bufferBarriers)
        {
            Dx12Buffer& dxBuffer = *api_cast<Dx12Buffer*>(bufferBarrier.BufferPtr);

            D3D12_RESOURCE_BARRIER barrier = {};
            D3D12_RESOURCE_STATES stateBefore = ResourceStateToD3D12ResourceStates(bufferBarrier.StateBefore);
            D3D12_RESOURCE_STATES stateAfter = ResourceStateToD3D12ResourceStates(bufferBarrier.StateAfter);
            
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.StateBefore = stateBefore;
            barrier.Transition.StateAfter = stateAfter;
            barrier.Transition.pResource = dxBuffer.GetD3D12Resource().Get();
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            resourceBarriers.push_back(barrier);
        }

        // Place barriers
        if (!resourceBarriers.empty())
            m_CommandList->ResourceBarrier(static_cast<uint32_t>(resourceBarriers.size()), resourceBarriers.data());

        imageBarriers.clear();
        bufferBarriers.clear();
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Object methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12CommandList::StartRenderpass(const RenderpassStartArgs& args)
    {
        OB_PROFILE("Dx12CommandList::StartRenderpass()");
        
        OB_ASSERT(args.Pass, "[Dx12CommandList] No Renderpass passed in.");

        Dx12Renderpass& renderpass = *api_cast<Dx12Renderpass*>(args.Pass);
        Framebuffer* framebuffer = args.Frame;
        if (!framebuffer)
        {
            OB_ASSERT((renderpass.GetFramebuffers().size() == m_Pool.GetDx12Swapchain().GetImageCount()), "[Dx12CommandList] No framebuffer was passed into GraphicsState, but renderpass' framebuffer count doesn't align with swapchain image count.");
            framebuffer = &renderpass.GetFramebuffer(static_cast<uint8_t>(m_Pool.GetDx12Swapchain().GetAcquiredImage()));
        }
        Dx12Framebuffer& dxFramebuffer = *api_cast<Dx12Framebuffer*>(framebuffer);

        Dx12Image* colourImage = api_cast<Dx12Image*>(dxFramebuffer.GetSpecification().ColourAttachment.ImagePtr);
        Dx12Image* depthImage = api_cast<Dx12Image*>(dxFramebuffer.GetSpecification().DepthAttachment.ImagePtr);

        // Make sure the attachments are in the begin state
        {
            if (framebuffer->GetSpecification().ColourAttachment.IsValid() && (renderpass.GetSpecification().ColourImageStartState != ResourceState::Unknown))
            {
                const FramebufferAttachment& attachment = framebuffer->GetSpecification().ColourAttachment;
                RequireState(*attachment.ImagePtr, attachment.Subresources, renderpass.GetSpecification().ColourImageStartState);
            }
            if (framebuffer->GetSpecification().DepthAttachment.IsValid() && (renderpass.GetSpecification().DepthImageStartState != ResourceState::Unknown))
            {
                const FramebufferAttachment& attachment = framebuffer->GetSpecification().DepthAttachment;
                RequireState(*attachment.ImagePtr, attachment.Subresources, renderpass.GetSpecification().DepthImageStartState);
            }
            CommitBarriers();
        }

        D3D12_RENDER_PASS_RENDER_TARGET_DESC colourDesc = {};
        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthDesc = {};

        if (colourImage)
        {
            colourDesc.cpuDescriptor = colourImage->GetSubresourceView(dxFramebuffer.GetSpecification().ColourAttachment.Subresources, ImageSubresourceViewUsage::RTV).GetCPUHandle();

            colourDesc.BeginningAccess.Type = LoadOperationToD3D12BeginningAccess(renderpass.GetSpecification().ColourLoadOperation);
            colourDesc.BeginningAccess.Clear.ClearValue.Format = FormatToFormatMapping(colourImage->GetSpecification().ImageFormat).RTVFormat;
            colourDesc.BeginningAccess.Clear.ClearValue.Color[0] = args.ColourClear.r;
            colourDesc.BeginningAccess.Clear.ClearValue.Color[1] = args.ColourClear.g;
            colourDesc.BeginningAccess.Clear.ClearValue.Color[2] = args.ColourClear.b;
            colourDesc.BeginningAccess.Clear.ClearValue.Color[3] = args.ColourClear.a;

            colourDesc.EndingAccess.Type = StoreOperationToD3D12EndingAccess(renderpass.GetSpecification().ColourStoreOperation);

            // Transition to rendering state
            if (renderpass.GetSpecification().ColourImageStartState != renderpass.GetSpecification().ColourImageRenderingState)
                RequireState(*api_cast<Image*>(colourImage), dxFramebuffer.GetSpecification().ColourAttachment.Subresources, renderpass.GetSpecification().ColourImageRenderingState);
        }
        if (depthImage)
        {
            depthDesc.cpuDescriptor = depthImage->GetSubresourceView(dxFramebuffer.GetSpecification().DepthAttachment.Subresources, ImageSubresourceViewUsage::DSV, ImageDimension::Image2D, Format::Unknown, false).GetCPUHandle();

            // Note: We currently don't support stencil
            depthDesc.DepthBeginningAccess.Type = LoadOperationToD3D12BeginningAccess(renderpass.GetSpecification().DepthLoadOperation);
            depthDesc.DepthBeginningAccess.Clear.ClearValue.Format = FormatToFormatMapping(depthImage->GetSpecification().ImageFormat).RTVFormat;
            depthDesc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = args.DepthClear;

            depthDesc.DepthEndingAccess.Type = StoreOperationToD3D12EndingAccess(renderpass.GetSpecification().DepthStoreOperation);

            // Transition to rendering state
            if (renderpass.GetSpecification().DepthImageStartState != renderpass.GetSpecification().DepthImageRenderingState)
                RequireState(*api_cast<Image*>(depthImage), dxFramebuffer.GetSpecification().DepthAttachment.Subresources, renderpass.GetSpecification().DepthImageRenderingState);
        }
        CommitBarriers();

        {
            OB_PROFILE("Dx12CommandList::SetGraphicsState::BeginRenderpass"); // Note: Only 1 render target is currently supported
            m_CommandList->BeginRenderPass((colourImage ? 1 : 0), (colourImage ? &colourDesc : nullptr), (depthImage ? &depthDesc : nullptr), D3D12_RENDER_PASS_FLAG_NONE);
        }

        SetViewport(args.ViewportState);
        SetScissor(args.Scissor);
    }

    void Dx12CommandList::EndRenderpass(const RenderpassEndArgs& args)
    {
        OB_PROFILE("Dx12CommandList::EndRenderpass()");

        m_CommandList->EndRenderPass();

        // Transition to FinalState
        {
            Dx12Renderpass& dxRenderpass = *api_cast<Dx12Renderpass*>(args.Pass);
            Framebuffer* framebuffer = args.Frame;
            if (!framebuffer)
            {
                OB_ASSERT((dxRenderpass.GetFramebuffers().size() == m_Pool.GetDx12Swapchain().GetImageCount()), "[Dx12CommandList] No framebuffer was passed into GraphicsState, but renderpass' framebuffer count doesn't align with swapchain image count.");
                framebuffer = &dxRenderpass.GetFramebuffer(static_cast<uint8_t>(m_Pool.GetDx12Swapchain().GetAcquiredImage()));
            }

            // Note: On Dx12 we need to manually transition to the EndState
            {
                if (framebuffer->GetSpecification().ColourAttachment.IsValid())
                {
                    const FramebufferAttachment& attachment = framebuffer->GetSpecification().ColourAttachment;
                    RequireState(*attachment.ImagePtr, attachment.Subresources, dxRenderpass.GetSpecification().ColourImageEndState);
                }
                if (framebuffer->GetSpecification().DepthAttachment.IsValid())
                {
                    const FramebufferAttachment& attachment = framebuffer->GetSpecification().DepthAttachment;
                    RequireState(*attachment.ImagePtr, attachment.Subresources, dxRenderpass.GetSpecification().DepthImageEndState);
                }
                CommitBarriers();
            }
        }
    }

    void Dx12CommandList::BindPipeline(const GraphicsPipeline& pipeline)
    {
        OB_PROFILE("Dx12CommandList::BindPipeline()");

        m_CurrentGraphicsPipeline = &pipeline;
        m_CurrentComputePipeline = nullptr;

        const Dx12GraphicsPipeline& dxPipeline = *api_cast<const Dx12GraphicsPipeline*>(m_CurrentGraphicsPipeline);

        // Bind pipeline
        m_CommandList->SetPipelineState(dxPipeline.GetD3D12PipelineState().Get());
        m_CommandList->SetGraphicsRootSignature(dxPipeline.GetD3D12RootSignature().Get());
        m_CommandList->IASetPrimitiveTopology(PrimitiveTypeToD3DPrimitiveTopology(dxPipeline.GetSpecification().Primitive, dxPipeline.GetSpecification().PatchPointCount));

        // Bind heaps for BindingSet(s)
        const auto& resources = m_Pool.GetDx12Swapchain().GetDx12Device().GetResources();

        auto heaps = std::to_array<ID3D12DescriptorHeap*>({ resources.GetSRVAndUAVAndCBVHeap().GetD3D12DescriptorHeap().Get(), resources.GetSamplerHeap().GetD3D12DescriptorHeap().Get() });
        m_CommandList->SetDescriptorHeaps(static_cast<UINT>(heaps.size()), heaps.data());
    }

    void Dx12CommandList::BindPipeline(const ComputePipeline& pipeline)
    {
        OB_PROFILE("Dx12CommandList::BindPipeline()");

        m_CurrentGraphicsPipeline = nullptr;
        m_CurrentComputePipeline = &pipeline;

        const Dx12GraphicsPipeline& dxPipeline = *api_cast<const Dx12GraphicsPipeline*>(m_CurrentGraphicsPipeline);

        // Bind pipeline
        m_CommandList->SetPipelineState(dxPipeline.GetD3D12PipelineState().Get());
        m_CommandList->SetComputeRootSignature(dxPipeline.GetD3D12RootSignature().Get());

        // Bind heaps for BindingSet(s)
        const auto& resources = m_Pool.GetDx12Swapchain().GetDx12Device().GetResources();

        auto heaps = std::to_array<ID3D12DescriptorHeap*>({ resources.GetSRVAndUAVAndCBVHeap().GetD3D12DescriptorHeap().Get(), resources.GetSamplerHeap().GetD3D12DescriptorHeap().Get() });
        m_CommandList->SetDescriptorHeaps(static_cast<UINT>(heaps.size()), heaps.data());
    }

    void Dx12CommandList::BindBindingSet(const BindingSet& set, std::span<const uint32_t> dynamicOffsets)
    {
        OB_PROFILE("Dx12CommandList::BindBindingSet()");

        OB_ASSERT(m_CurrentGraphicsPipeline || m_CurrentComputePipeline, "[Dx12CommandList] A pipeline must be bound to bind bindingsets.");
        const Dx12BindingSet& dxSet = *api_cast<const Dx12BindingSet*>(&set);

        // Graphics pipeline
        if (m_CurrentGraphicsPipeline)
        {
            const Dx12GraphicsPipeline& graphicsPipeline = *api_cast<const Dx12GraphicsPipeline*>(m_CurrentGraphicsPipeline);
            const Dx12BindingLayout& layout = *api_cast<const Dx12BindingLayout*>(dxSet.GetDx12BindingSetPool().GetSpecification().Layout);

            const auto& srvAndUAVandCBVRootIndices = graphicsPipeline.GetSRVAndUAVAndCBVRootIndices(layout.GetBindingSpecification().RegisterSpace);
            const auto& samplerRootIndices = graphicsPipeline.GetSamplerRootIndices(layout.GetBindingSpecification().RegisterSpace);
            const auto& dynamicRootIndices = graphicsPipeline.GetDynamicRootIndices(layout.GetBindingSpecification().RegisterSpace);

            if constexpr (Information::Validation)
            {
                const auto& srvAndUAVAndCBVRanges = layout.GetSRVAndUAVAndCBVRanges();
                const auto& samplerRanges = layout.GetSamplerRanges();
                const auto& dynamicRanges = layout.GetDynamicRanges();

                if (!srvAndUAVAndCBVRanges.empty() && (srvAndUAVandCBVRootIndices.empty()))
                    OB_ASSERT(false, "[Dx12CommandList] Internal error: Something went very wrong, the ranges aren't empty but there was no root parameter created for it.");
                if (!samplerRanges.empty() && (samplerRootIndices.empty()))
                    OB_ASSERT(false, "[Dx12CommandList] Internal error: Something went very wrong, the ranges aren't empty but there was no root parameter created for it.");
                if (!dynamicRanges.empty() && (dynamicRootIndices.empty()))
                    OB_ASSERT(false, "[Dx12CommandList] Internal error: Something went very wrong, the ranges aren't empty but there was no root parameter created for it.");
            }

            const auto& resources = m_Pool.GetDx12Swapchain().GetDx12Device().GetResources();

            // SRVs, UAVs & CBVs
            for (const auto& [slot, index] : srvAndUAVandCBVRootIndices)
            {
                CD3DX12_GPU_DESCRIPTOR_HANDLE handle = resources.GetSRVAndUAVAndCBVHeap().GetGPUHandleForIndex(dxSet.GetSRVAndUAVAndCBVBeginIndex() + layout.GetSlotToHeapOffset(slot));
                m_CommandList->SetGraphicsRootDescriptorTable(index, handle);
            }

            // Samplers
            for (const auto& [slot, index] : samplerRootIndices)
            {
                CD3DX12_GPU_DESCRIPTOR_HANDLE handle = resources.GetSamplerHeap().GetGPUHandleForIndex(dxSet.GetSamplerBeginIndex() + layout.GetSlotToHeapOffset(slot));
                m_CommandList->SetGraphicsRootDescriptorTable(index, handle);
            }

            // Dynamic
            size_t currentOffset = 0;
            for (size_t i = 0; i < dynamicRootIndices.size(); i++)
            {
                auto& indexInfo = dynamicRootIndices[i];
                OB_ASSERT((ResourceTypeIsDynamic(indexInfo.Type)), "[Dx12CommandList] Internal error: A non dynamic resource was passed into dynamic root indices.");

                uint32_t offset = ((dynamicOffsets.empty()) ? 0 : dynamicOffsets[currentOffset++]);

                switch (indexInfo.Type)
                {
                case ResourceType::DynamicStructuredBufferSRV:
                    m_CommandList->SetGraphicsRootShaderResourceView(indexInfo.Index, dxSet.GetD3D12GPUAddressForDynamicSlot(indexInfo.Slot) + offset);
                    break;
                case ResourceType::DynamicStructuredBufferUAV:
                    m_CommandList->SetGraphicsRootUnorderedAccessView(indexInfo.Index, dxSet.GetD3D12GPUAddressForDynamicSlot(indexInfo.Slot) + offset);
                    break;
                case ResourceType::DynamicConstantBuffer:
                    m_CommandList->SetGraphicsRootConstantBufferView(indexInfo.Index, dxSet.GetD3D12GPUAddressForDynamicSlot(indexInfo.Slot) + offset);
                    break;

                default:
                    OB_UNREACHABLE();
                    break;
                }
            }
        }
        // Compute pipeline
        else
        {
            const Dx12ComputePipeline& computePipeline = *api_cast<const Dx12ComputePipeline*>(m_CurrentComputePipeline);
            const Dx12BindingLayout& layout = *api_cast<const Dx12BindingLayout*>(dxSet.GetDx12BindingSetPool().GetSpecification().Layout);

            const auto& srvAndUAVandCBVRootIndices = computePipeline.GetSRVAndUAVAndCBVRootIndices(layout.GetBindingSpecification().RegisterSpace);
            const auto& samplerRootIndices = computePipeline.GetSamplerRootIndices(layout.GetBindingSpecification().RegisterSpace);
            const auto& dynamicRootIndices = computePipeline.GetDynamicRootIndices(layout.GetBindingSpecification().RegisterSpace);

            if constexpr (Information::Validation)
            {
                const auto& srvAndUAVAndCBVRanges = layout.GetSRVAndUAVAndCBVRanges();
                const auto& samplerRanges = layout.GetSamplerRanges();
                const auto& dynamicRanges = layout.GetDynamicRanges();

                if (!srvAndUAVAndCBVRanges.empty() && (srvAndUAVandCBVRootIndices.empty()))
                    OB_ASSERT(false, "[Dx12CommandList] Internal error: Something went very wrong, the ranges aren't empty but there was no root parameter created for it.");
                if (!samplerRanges.empty() && (samplerRootIndices.empty()))
                    OB_ASSERT(false, "[Dx12CommandList] Internal error: Something went very wrong, the ranges aren't empty but there was no root parameter created for it.");
                if (!dynamicRanges.empty() && (dynamicRootIndices.empty()))
                    OB_ASSERT(false, "[Dx12CommandList] Internal error: Something went very wrong, the ranges aren't empty but there was no root parameter created for it.");
            }

            const auto& resources = m_Pool.GetDx12Swapchain().GetDx12Device().GetResources();

            // SRVs, UAVs & CBVs
            for (const auto& [slot, index] : srvAndUAVandCBVRootIndices)
            {
                CD3DX12_GPU_DESCRIPTOR_HANDLE handle = resources.GetSRVAndUAVAndCBVHeap().GetGPUHandleForIndex(dxSet.GetSRVAndUAVAndCBVBeginIndex() + layout.GetSlotToHeapOffset(slot));
                m_CommandList->SetGraphicsRootDescriptorTable(index, handle);
            }

            // Samplers
            for (const auto& [slot, index] : samplerRootIndices)
            {
                CD3DX12_GPU_DESCRIPTOR_HANDLE handle = resources.GetSamplerHeap().GetGPUHandleForIndex(dxSet.GetSamplerBeginIndex() + layout.GetSlotToHeapOffset(slot));
                m_CommandList->SetComputeRootDescriptorTable(index, handle);
            }

            // Dynamic
            size_t currentOffset = 0;
            for (size_t i = 0; i < dynamicRootIndices.size(); i++)
            {
                auto& indexInfo = dynamicRootIndices[i];
                OB_ASSERT((ResourceTypeIsDynamic(indexInfo.Type)), "[Dx12CommandList] Internal error: A non dynamic resource was passed into dynamic root indices.");

                uint32_t offset = ((dynamicOffsets.empty()) ? 0 : dynamicOffsets[currentOffset++]);

                switch (indexInfo.Type)
                {
                case ResourceType::DynamicStructuredBufferSRV:
                    m_CommandList->SetComputeRootShaderResourceView(indexInfo.Index, dxSet.GetD3D12GPUAddressForDynamicSlot(indexInfo.Slot) + offset);
                    break;
                case ResourceType::DynamicStructuredBufferUAV:
                    m_CommandList->SetComputeRootUnorderedAccessView(indexInfo.Index, dxSet.GetD3D12GPUAddressForDynamicSlot(indexInfo.Slot) + offset);
                    break;
                case ResourceType::DynamicConstantBuffer:
                    m_CommandList->SetComputeRootConstantBufferView(indexInfo.Index, dxSet.GetD3D12GPUAddressForDynamicSlot(indexInfo.Slot) + offset);
                    break;

                default:
                    OB_UNREACHABLE();
                    break;
                }
            }
        }
    }

    void Dx12CommandList::BindBindingSets(std::span<const BindingSet*> sets, std::span<const std::span<const uint32_t>> dynamicOffsets)
    {
        OB_PROFILE("Dx12CommandList::BindBindingSets()");

        OB_ASSERT((dynamicOffsets.empty()) || (sets.size() == dynamicOffsets.size()), "[Dx12CommandList] The amount of dynamic offsets spans must be the same as the amount of sets or be empty.");

        for (size_t i = 0; i < sets.size(); i++)
            BindBindingSet(*sets[i], (!dynamicOffsets.empty() ? dynamicOffsets[i] : std::span<const uint32_t>()));
    }

    void Dx12CommandList::SetViewport(const Viewport& viewport) const
    {
        OB_PROFILE("Dx12CommandList::SetViewport()");

        // Note: We use VK coordinates
        D3D12_VIEWPORT d3d12Viewport = {};
        d3d12Viewport.TopLeftX = viewport.MinX;
        d3d12Viewport.TopLeftY = viewport.MinY + viewport.GetHeight(); // Shift origin to top-left
        d3d12Viewport.Width = viewport.GetWidth();
        d3d12Viewport.Height = -viewport.GetHeight(); // Flip Y-axis
        d3d12Viewport.MinDepth = viewport.MinZ;
        d3d12Viewport.MaxDepth = viewport.MaxZ;

        m_CommandList->RSSetViewports(1, &d3d12Viewport);
    }

    void Dx12CommandList::SetScissor(const ScissorRect& scissor) const
    {
        OB_PROFILE("Dx12CommandList::SetScissor()");

        D3D12_RECT rect = {};
        rect.left = scissor.MinX;
        rect.top = scissor.MinY;
        rect.right = scissor.MaxX;
        rect.bottom = scissor.MaxY;

        m_CommandList->RSSetScissorRects(1, &rect);
    }

    void Dx12CommandList::BindVertexBuffer(const Buffer& buffer) const
    {
        OB_PROFILE("Dx12CommandList::BindVertexBuffer()");

        OB_ASSERT(buffer.GetSpecification().IsVertexBuffer, "[Dx12CommandList] Buffer must have IsVertexBuffer flag to be bound as a vertex buffer.");
        OB_ASSERT(m_CurrentGraphicsPipeline, "[Dx12CommandList] Can't bind vertexbuffer if no pipeline is bound.");

        const Dx12Buffer& dxBuffer = *api_cast<const Dx12Buffer*>(&buffer);
        const Dx12InputLayout& dxLayout = *api_cast<const Dx12InputLayout*>(m_CurrentGraphicsPipeline->GetSpecification().Input);

        D3D12_VERTEX_BUFFER_VIEW view = {};
        view.BufferLocation = dxBuffer.GetD3D12Resource()->GetGPUVirtualAddress();
        view.SizeInBytes = static_cast<UINT>(dxBuffer.GetSpecification().Size);
        view.StrideInBytes = dxLayout.GetStride();

        m_CommandList->IASetVertexBuffers(0, 1, &view);
    }

    void Dx12CommandList::BindIndexBuffer(const Buffer& buffer) const
    {
        OB_PROFILE("Dx12CommandList::BindIndexBuffer()");

        OB_ASSERT(buffer.GetSpecification().IsIndexBuffer, "[Dx12CommandList] Buffer must have IsIndexBuffer flag to be bound as an index buffer.");

        const Dx12Buffer& dxBuffer = *api_cast<const Dx12Buffer*>(&buffer);

        D3D12_INDEX_BUFFER_VIEW view = {};
        view.BufferLocation = dxBuffer.GetD3D12Resource()->GetGPUVirtualAddress();
        view.SizeInBytes = static_cast<UINT>(dxBuffer.GetSpecification().Size);
        view.Format = FormatToFormatMapping(dxBuffer.GetSpecification().BufferFormat).SRVFormat;

        m_CommandList->IASetIndexBuffer(&view);
    }

    void Dx12CommandList::CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, Image& src, const ImageSliceSpecification& srcSlice)
    {
        OB_PROFILE("Dx12CommandList::CopyImage()");

        Dx12Image& dxDst = *api_cast<Dx12Image*>(&dst);
        Dx12Image& dxSrc = *api_cast<Dx12Image*>(&src);

        ImageSliceSpecification resDstSlice = ResolveImageSlice(dstSlice, dst.GetSpecification());
        ImageSliceSpecification resSrcSlice = ResolveImageSlice(srcSlice, src.GetSpecification());

        UINT dstSubresource = CalculateSubresource(resDstSlice.ImageMipLevel, resDstSlice.ImageArraySlice, 0, dst.GetSpecification().MipLevels, dst.GetSpecification().ArraySize);
        UINT srcSubresource = CalculateSubresource(resSrcSlice.ImageMipLevel, resSrcSlice.ImageArraySlice, 0, src.GetSpecification().MipLevels, src.GetSpecification().ArraySize);

        ImageSubresourceSpecification srcSubresourceSpec = ImageSubresourceSpecification(
            resSrcSlice.ImageMipLevel, 1,
            resSrcSlice.ImageArraySlice, 1
        );

        ImageSubresourceSpecification dstSubresourceSpec = ImageSubresourceSpecification(
            resDstSlice.ImageMipLevel, 1,
            resDstSlice.ImageArraySlice, 1
        );

        D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
        dstLocation.pResource = dxDst.GetD3D12Resource().Get();
        dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLocation.SubresourceIndex = dstSubresource;

        D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
        srcLocation.pResource = dxSrc.GetD3D12Resource().Get();
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        srcLocation.SubresourceIndex = srcSubresource;

        D3D12_BOX srcBox;
        srcBox.left = resSrcSlice.X;
        srcBox.top = resSrcSlice.Y;
        srcBox.front = resSrcSlice.Z;
        srcBox.right = resSrcSlice.X + resSrcSlice.Width;
        srcBox.bottom = resSrcSlice.Y + resSrcSlice.Height;
        srcBox.back = resSrcSlice.Z + resSrcSlice.Depth;

        RequireState(dst, ImageSubresourceSpecification(resDstSlice.ImageMipLevel, 1, resDstSlice.ImageArraySlice, 1), ResourceState::CopyDst);
        RequireState(src, ImageSubresourceSpecification(resSrcSlice.ImageMipLevel, 1, resSrcSlice.ImageArraySlice, 1), ResourceState::CopySrc);
        CommitBarriers();

        m_CommandList->CopyTextureRegion(&dstLocation, resDstSlice.X, resDstSlice.Y, resDstSlice.Z, &srcLocation, &srcBox);

        // Update back to permanent state
        m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().ResolvePermanentState(*api_cast<const CommandList*>(this), src, srcSubresourceSpec);
        m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().ResolvePermanentState(*api_cast<const CommandList*>(this), dst, dstSubresourceSpec);
        CommitBarriers();
    }

    void Dx12CommandList::CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, StagingImage& src, const ImageSliceSpecification& srcSlice)
    {
        OB_PROFILE("Dx12CommandList::CopyImage()");

        Dx12StagingImage& dxSrc = *api_cast<Dx12StagingImage*>(&src);
        Dx12Image& dxDst = *api_cast<Dx12Image*>(&dst);

        ImageSliceSpecification resDstSlice = ResolveImageSlice(dstSlice, dst.GetSpecification());
        ImageSliceSpecification resSrcSlice = ResolveImageSlice(srcSlice, src.GetSpecification());

        UINT dstSubresource = CalculateSubresource(resDstSlice.ImageMipLevel, resDstSlice.ImageArraySlice, 0, dst.GetSpecification().MipLevels, dst.GetSpecification().ArraySize);

        ImageSubresourceSpecification dstSubresourceSpec = ImageSubresourceSpecification(
            resDstSlice.ImageMipLevel, 1,
            resDstSlice.ImageArraySlice, 1
        );

        RequireState(dst, ImageSubresourceSpecification(resDstSlice.ImageMipLevel, 1, resDstSlice.ImageArraySlice, 1), ResourceState::CopyDst);
        RequireState(*api_cast<Buffer*>(&dxSrc.GetDx12Buffer()), ResourceState::CopySrc);
        CommitBarriers();

        auto srcRegion = dxSrc.GetSliceRegion(resSrcSlice.ImageMipLevel, resSrcSlice.ImageArraySlice);

        D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
        dstLocation.pResource = dxDst.GetD3D12Resource().Get();
        dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLocation.SubresourceIndex = dstSubresource;

        D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
        srcLocation.pResource = dxSrc.GetDx12Buffer().GetD3D12Resource().Get();
        srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        srcLocation.PlacedFootprint = srcRegion.Footprint;

        D3D12_BOX srcBox = {};
        srcBox.left = resSrcSlice.X;
        srcBox.top = resSrcSlice.Y;
        srcBox.front = resSrcSlice.Z;
        srcBox.right = resSrcSlice.X + resSrcSlice.Width;
        srcBox.bottom = resSrcSlice.Y + resSrcSlice.Height;
        srcBox.back = resSrcSlice.Z + resSrcSlice.Depth;

        m_CommandList->CopyTextureRegion(&dstLocation, resDstSlice.X, resDstSlice.Y, resDstSlice.Z, &srcLocation, &srcBox);

        // Update back to permanent state
        m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().ResolvePermanentState(*api_cast<const CommandList*>(this), *api_cast<Buffer*>(&dxSrc.GetDx12Buffer()));
        m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().ResolvePermanentState(*api_cast<const CommandList*>(this), *api_cast<Image*>(&dxDst), dstSubresourceSpec);
        CommitBarriers();
    }

    void Dx12CommandList::CopyBuffer(Buffer& dst, Buffer& src, size_t size, size_t srcOffset, size_t dstOffset)
    {
        OB_PROFILE("Dx12CommandList::CopyBuffer()");

        Dx12Buffer& dxDst = *api_cast<Dx12Buffer*>(&dst);
        Dx12Buffer& dxSrc = *api_cast<Dx12Buffer*>(&src);

        RequireState(dst, ResourceState::CopyDst);
        RequireState(src, ResourceState::CopySrc);
        CommitBarriers();

        m_CommandList->CopyBufferRegion(dxDst.GetD3D12Resource().Get(), dstOffset, dxSrc.GetD3D12Resource().Get(), srcOffset, size);
    }

    void Dx12CommandList::Dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) const
    {
        OB_PROFILE("Dx12CommandList::Dispatch()");
        m_CommandList->Dispatch(groupsX, groupsY, groupsZ);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // State methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12CommandList::RequireState(Image& image, const ImageSubresourceSpecification& subresources, ResourceState state)
    {
        m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().RequireImageState(*api_cast<const CommandList*>(this), image, subresources, state);
    }

    void Dx12CommandList::RequireState(Buffer& buffer, ResourceState state)
    {
        m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().RequireBufferState(*api_cast<const CommandList*>(this), buffer, state);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Draw methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12CommandList::DrawIndexed(const DrawArguments& args) const
    {
        OB_PROFILE("Dx12CommandList::DrawIndexed()");
        m_CommandList->DrawIndexedInstanced(args.VertexCount, args.InstanceCount, args.StartIndexLocation, args.StartVertexLocation, args.StartInstanceLocation);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Other methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12CommandList::PushConstants(const void* memory, size_t size, size_t srcOffset, size_t dstOffset)
    {
        OB_PROFILE("Dx12CommandList::PushConstants()");

        OB_ASSERT(m_CurrentGraphicsPipeline || m_CurrentComputePipeline, "[Dx12CommandList] Can't push constants if no pipeline is bound.");
        OB_ASSERT((size + dstOffset <= 128), "[Dx12CommandList] Size + dstOffset exceeds max push constants size (128 bytes)");
        OB_ASSERT((dstOffset % 4 == 0), "[Dx12CommandList] DstOffset must be aligned to 4 bytes.");
        OB_ASSERT((size % 4 == 0), "[Dx12CommandList] Size must be aligned to 4 bytes.");

        if (m_CurrentGraphicsPipeline)
        {
            const Dx12GraphicsPipeline& dxPipeline = *api_cast<const Dx12GraphicsPipeline*>(m_CurrentGraphicsPipeline);
            OB_ASSERT((dxPipeline.GetPushConstantsRootIndex().Index != Dx12GraphicsPipeline::RootParameterIndices::Invalid), "[Dx12CommandList] Trying to push constants with no root parameter created for constants.");
            m_CommandList->SetGraphicsRoot32BitConstants(dxPipeline.GetPushConstantsRootIndex().Index, static_cast<UINT>(size / 4), static_cast<const uint8_t*>(memory) + srcOffset, static_cast<UINT>(dstOffset / 4));
        }
        else
        {
            const Dx12ComputePipeline& dxPipeline = *api_cast<const Dx12ComputePipeline*>(m_CurrentComputePipeline);
            OB_ASSERT((dxPipeline.GetPushConstantsRootIndex().Index != Dx12ComputePipeline::RootParameterIndices::Invalid), "[Dx12CommandList] Trying to push constants with no root parameter created for constants.");
            m_CommandList->SetComputeRoot32BitConstants(dxPipeline.GetPushConstantsRootIndex().Index, static_cast<UINT>(size / 4), static_cast<const uint8_t*>(memory) + srcOffset, static_cast<UINT>(dstOffset / 4));
        }
    }

}