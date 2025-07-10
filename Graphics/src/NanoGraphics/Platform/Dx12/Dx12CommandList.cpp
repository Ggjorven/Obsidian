#include "ngpch.h"
#include "Dx12CommandList.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Core/Window.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/CommandList.hpp"
#include "NanoGraphics/Renderer/Swapchain.hpp"
#include "NanoGraphics/Renderer/Renderpass.hpp"
#include "NanoGraphics/Renderer/Framebuffer.hpp"
#include "NanoGraphics/Renderer/Buffer.hpp"
#include "NanoGraphics/Renderer/Image.hpp"
#include "NanoGraphics/Renderer/Pipeline.hpp"
#include "NanoGraphics/Renderer/Bindings.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12Device.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Buffer.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Image.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Resources.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Renderpass.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Framebuffer.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Pipeline.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Bindings.hpp"

namespace Nano::Graphics::Internal
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
                NG_UNREACHABLE();
                break;
            }

            return D3D12_COMMAND_LIST_TYPE_DIRECT;
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
        NG_PROFILE("Dx12CommandList::Open()");
        DX_VERIFY(m_CommandList->Reset(m_Pool.GetD3D12CommandAllocator().Get(), nullptr));
    }

    void Dx12CommandList::Close()
    {
        NG_PROFILE("Dx12CommandList::Close()");
        DX_VERIFY(m_CommandList->Close());

        m_CurrentGraphicsPipeline = nullptr;
    }

    void Dx12CommandList::Submit(const CommandListSubmitArgs& args)
    {
        NG_PROFILE("VulkanCommandBuffer::Submit()");

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

        // Note: Waiting on swapchain image is not a thing that needs to handled manually for DX12
        
        ID3D12CommandList* lists[] = { m_CommandList.Get() };
        queue->ExecuteCommandLists(1, lists);

        m_SignaledValue = m_Pool.GetDx12Swapchain().RetrieveCommandListWaitValue(*this);
        DX_VERIFY(queue->Signal(m_Pool.GetDx12Swapchain().GetD3D12Fence().Get(), m_SignaledValue));
        
        if (args.OnFinishMakeSwapchainPresentable)
            m_Pool.GetDx12Swapchain().SetPresentableValue(m_SignaledValue);
    }

    void Dx12CommandList::WaitTillComplete() const
    {
        NG_PROFILE("Dx12CommandList::WaitTillComplete()");

        DxPtr<ID3D12CommandQueue> queue = m_Pool.GetDx12Swapchain().GetDx12Device().GetContext().GetD3D12CommandQueue(m_Pool.GetSpecification().Queue);
        DxPtr<ID3D12Fence> fence = m_Pool.GetDx12Swapchain().GetD3D12Fence();

        fence->SetEventOnCompletion(m_SignaledValue, m_WaitIdleEvent);
        WaitForSingleObject(m_WaitIdleEvent, INFINITE);
    }

    void Dx12CommandList::CommitBarriers()
    {
        NG_PROFILE("Dx12CommandList::CommitBarriers()");

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
        NG_PROFILE("Dx12CommandList::StartRenderpass()");
        
        NG_ASSERT(args.Pass, "[Dx12CommandList] No Renderpass passed in.");

        Dx12Renderpass& renderpass = *api_cast<Dx12Renderpass*>(args.Pass);
        Framebuffer* framebuffer = args.Frame;
        if (!framebuffer)
        {
            NG_ASSERT((renderpass.GetFramebuffers().size() == m_Pool.GetDx12Swapchain().GetImageCount()), "[Dx12CommandList] No framebuffer was passed into GraphicsState, but renderpass' framebuffer count doesn't align with swapchain image count.");
            framebuffer = &renderpass.GetFramebuffer(static_cast<uint8_t>(m_Pool.GetDx12Swapchain().GetAcquiredImage()));
        }
        Dx12Framebuffer& dxFramebuffer = *api_cast<Dx12Framebuffer*>(framebuffer);

        Dx12Image* colourImage = api_cast<Dx12Image*>(dxFramebuffer.GetSpecification().ColourAttachment.ImagePtr);
        Dx12Image* depthImage = api_cast<Dx12Image*>(dxFramebuffer.GetSpecification().DepthAttachment.ImagePtr);

        // Validation checks
        if constexpr (Information::Validation)
        {
            if (colourImage)
            {
                ResourceState currentState = m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().GetResourceState(*api_cast<Image*>(colourImage), dxFramebuffer.GetSpecification().ColourAttachment.Subresources);
                ResourceState startState = renderpass.GetSpecification().ColourImageStartState;
                if (currentState != startState)
                    m_Pool.GetDx12Swapchain().GetDx12Device().GetContext().Error(std::format("[Dx12CommandList] Current colour image state ({0}) doesn't match the renderpass' specified colour image start state ({1}).", ResourceStateToString(currentState), ResourceStateToString(startState)));
            }
            if (depthImage)
            {
                ResourceState currentState = m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().GetResourceState(*api_cast<Image*>(depthImage), dxFramebuffer.GetSpecification().DepthAttachment.Subresources);
                ResourceState startState = renderpass.GetSpecification().DepthImageStartState;
                if (currentState != startState)
                    m_Pool.GetDx12Swapchain().GetDx12Device().GetContext().Error(std::format("[Dx12CommandList] Current depth image state ({0}) doesn't match the renderpass' specified depth image start state ({1}).", ResourceStateToString(currentState), ResourceStateToString(startState)));
            }
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
                m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().RequireImageState(*api_cast<const CommandList*>(this), *api_cast<Image*>(colourImage), dxFramebuffer.GetSpecification().ColourAttachment.Subresources, renderpass.GetSpecification().ColourImageRenderingState);
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
                m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().RequireImageState(*api_cast<const CommandList*>(this), *api_cast<Image*>(depthImage), dxFramebuffer.GetSpecification().DepthAttachment.Subresources, renderpass.GetSpecification().DepthImageRenderingState);
        }
        CommitBarriers();

        {
            NG_PROFILE("Dx12CommandList::SetGraphicsState::BeginRenderpass"); // Note: Only 1 render target is currently supported
            m_CommandList->BeginRenderPass((colourImage ? 1 : 0), (colourImage ? &colourDesc : nullptr), (depthImage ? &depthDesc : nullptr), D3D12_RENDER_PASS_FLAG_NONE);
        }

        SetViewport(args.ViewportState);
        SetScissor(args.Scissor);
    }

    void Dx12CommandList::EndRenderpass(const RenderpassEndArgs& args)
    {
        NG_PROFILE("Dx12CommandList::EndRenderpass()");

        m_CommandList->EndRenderPass();

        // Transition to FinalState
        {
            Dx12Renderpass& dxRenderpass = *api_cast<Dx12Renderpass*>(args.Pass);
            Framebuffer* framebuffer = args.Frame;
            if (!framebuffer)
            {
                NG_ASSERT((dxRenderpass.GetFramebuffers().size() == m_Pool.GetDx12Swapchain().GetImageCount()), "[Dx12CommandList] No framebuffer was passed into GraphicsState, but renderpass' framebuffer count doesn't align with swapchain image count.");
                framebuffer = &dxRenderpass.GetFramebuffer(static_cast<uint8_t>(m_Pool.GetDx12Swapchain().GetAcquiredImage()));
            }
            Dx12Framebuffer& dxFramebuffer = *api_cast<Dx12Framebuffer*>(framebuffer);

            if (dxFramebuffer.GetSpecification().ColourAttachment.IsValid())
                m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().RequireImageState(*api_cast<const CommandList*>(this), *dxFramebuffer.GetSpecification().ColourAttachment.ImagePtr, dxFramebuffer.GetSpecification().ColourAttachment.Subresources, dxRenderpass.GetSpecification().ColourImageEndState);
            if (dxFramebuffer.GetSpecification().DepthAttachment.IsValid())
                m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().RequireImageState(*api_cast<const CommandList*>(this), *dxFramebuffer.GetSpecification().DepthAttachment.ImagePtr, dxFramebuffer.GetSpecification().DepthAttachment.Subresources, dxRenderpass.GetSpecification().DepthImageEndState);
            CommitBarriers();
        }
    }

    void Dx12CommandList::BindPipeline(const GraphicsPipeline& pipeline)
    {
        NG_PROFILE("Dx12CommandList::BindPipeline()");

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
        NG_PROFILE("Dx12CommandList::BindPipeline()");

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

    void Dx12CommandList::BindBindingSet(const BindingSet& set)
    {
        NG_PROFILE("Dx12CommandList::BindBindingSet()");

        NG_ASSERT(m_CurrentGraphicsPipeline || m_CurrentComputePipeline, "[Dx12CommandList] A pipeline must be bound to bind bindingsets.");
        const Dx12BindingSet& dxSet = *api_cast<const Dx12BindingSet*>(&set);

        // Graphics pipeline
        if (m_CurrentGraphicsPipeline)
        {
            const Dx12GraphicsPipeline& graphicsPipeline = *api_cast<const Dx12GraphicsPipeline*>(m_CurrentGraphicsPipeline);
            const Dx12BindingLayout& layout = *api_cast<const Dx12BindingLayout*>(dxSet.GetDx12BindingSetPool().GetSpecification().Layout);

            const auto& srvAndUAVandCBVRootIndices = graphicsPipeline.GetSRVAndUAVAndCBVRootIndices(layout.GetBindingSpecification().RegisterSpace);
            const auto& samplerRootIndices = graphicsPipeline.GetSamplerRootIndices(layout.GetBindingSpecification().RegisterSpace);

            if constexpr (Information::Validation)
            {
                const auto& srvAndUAVAndCBVRanges = layout.GetD3D12SRVAndUAVAndCBVRanges();
                const auto& samplerRanges = layout.GetD3D12SamplerRanges();

                if (!srvAndUAVAndCBVRanges.empty() && (srvAndUAVandCBVRootIndices.empty()))
                    NG_ASSERT(false, "[Dx12CommandList] Internal error: Something went very wrong, the ranges aren't empty but there was no root parameter created for it.");
                if (!samplerRanges.empty() && (samplerRootIndices.empty()))
                    NG_ASSERT(false, "[Dx12CommandList] Internal error: Something went very wrong, the ranges aren't empty but there was no root parameter created for it.");
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
        }
        // Compute pipeline
        else
        {
            const Dx12ComputePipeline& computePipeline = *api_cast<const Dx12ComputePipeline*>(m_CurrentComputePipeline);
            const Dx12BindingLayout& layout = *api_cast<const Dx12BindingLayout*>(dxSet.GetDx12BindingSetPool().GetSpecification().Layout);

            const auto& srvAndUAVandCBVRootIndices = computePipeline.GetSRVAndUAVAndCBVRootIndices(layout.GetBindingSpecification().RegisterSpace);
            const auto& samplerRootIndices = computePipeline.GetSamplerRootIndices(layout.GetBindingSpecification().RegisterSpace);

            if constexpr (Information::Validation)
            {
                const auto& srvAndUAVAndCBVRanges = layout.GetD3D12SRVAndUAVAndCBVRanges();
                const auto& samplerRanges = layout.GetD3D12SamplerRanges();

                if (!srvAndUAVAndCBVRanges.empty() && (srvAndUAVandCBVRootIndices.empty()))
                    NG_ASSERT(false, "[Dx12CommandList] Internal error: Something went very wrong, the ranges aren't empty but there was no root parameter created for it.");
                if (!samplerRanges.empty() && (samplerRootIndices.empty()))
                    NG_ASSERT(false, "[Dx12CommandList] Internal error: Something went very wrong, the ranges aren't empty but there was no root parameter created for it.");
            }

            const auto& resources = m_Pool.GetDx12Swapchain().GetDx12Device().GetResources();

            // SRVs, UAVs & CBVs
            for (const auto& [slot, index] : srvAndUAVandCBVRootIndices)
            {
                CD3DX12_GPU_DESCRIPTOR_HANDLE handle = resources.GetSRVAndUAVAndCBVHeap().GetGPUHandleForIndex(dxSet.GetSRVAndUAVAndCBVBeginIndex() + layout.GetSlotToHeapOffset(slot));
                m_CommandList->SetComputeRootDescriptorTable(index, handle);
            }

            // Samplers
            for (const auto& [slot, index] : samplerRootIndices)
            {
                CD3DX12_GPU_DESCRIPTOR_HANDLE handle = resources.GetSamplerHeap().GetGPUHandleForIndex(dxSet.GetSamplerBeginIndex() + layout.GetSlotToHeapOffset(slot));
                m_CommandList->SetComputeRootDescriptorTable(index, handle);
            }
        }
    }

    void Dx12CommandList::BindBindingSets(std::span<const BindingSet*> sets)
    {
        NG_PROFILE("Dx12CommandList::BindBindingSets()");

        for (const auto set : sets)
            BindBindingSet(*set);
    }

    void Dx12CommandList::SetViewport(const Viewport& viewport) const
    {
        NG_PROFILE("Dx12CommandList::SetViewport()");

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
        NG_PROFILE("Dx12CommandList::SetScissor()");

        D3D12_RECT rect = {};
        rect.left = scissor.MinX;
        rect.top = scissor.MinY;
        rect.right = scissor.MaxX;
        rect.bottom = scissor.MaxY;

        m_CommandList->RSSetScissorRects(1, &rect);
    }

    void Dx12CommandList::BindVertexBuffer(const Buffer& buffer) const
    {
        NG_PROFILE("Dx12CommandList::BindVertexBuffer()");

        NG_ASSERT(buffer.GetSpecification().IsVertexBuffer, "[Dx12CommandList] Buffer must have IsVertexBuffer flag to be bound as a vertex buffer.");
        NG_ASSERT(m_CurrentGraphicsPipeline, "[Dx12CommandList] Can't bind vertexbuffer if no pipeline is bound.");

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
        NG_PROFILE("Dx12CommandList::BindIndexBuffer()");

        NG_ASSERT(buffer.GetSpecification().IsIndexBuffer, "[Dx12CommandList] Buffer must have IsIndexBuffer flag to be bound as an index buffer.");

        const Dx12Buffer& dxBuffer = *api_cast<const Dx12Buffer*>(&buffer);

        D3D12_INDEX_BUFFER_VIEW view = {};
        view.BufferLocation = dxBuffer.GetD3D12Resource()->GetGPUVirtualAddress();
        view.SizeInBytes = static_cast<UINT>(dxBuffer.GetSpecification().Size);
        view.Format = FormatToFormatMapping(dxBuffer.GetSpecification().BufferFormat).SRVFormat;

        m_CommandList->IASetIndexBuffer(&view);
    }

    void Dx12CommandList::CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, Image& src, const ImageSliceSpecification& srcSlice)
    {
        NG_PROFILE("Dx12CommandList::CopyImage()");

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

        m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().RequireImageState(*api_cast<const CommandList*>(this), dst, ImageSubresourceSpecification(resDstSlice.ImageMipLevel, 1, resDstSlice.ImageArraySlice, 1), ResourceState::CopyDst);
        m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().RequireImageState(*api_cast<const CommandList*>(this), src, ImageSubresourceSpecification(resSrcSlice.ImageMipLevel, 1, resSrcSlice.ImageArraySlice, 1), ResourceState::CopySrc);
        CommitBarriers();

        m_CommandList->CopyTextureRegion(&dstLocation, resDstSlice.X, resDstSlice.Y, resDstSlice.Z, &srcLocation, &srcBox);

        // Update back to permanent state
        m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().ResolvePermanentState(*api_cast<const CommandList*>(this), src, srcSubresourceSpec);
        m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().ResolvePermanentState(*api_cast<const CommandList*>(this), dst, dstSubresourceSpec);
        CommitBarriers();
    }

    void Dx12CommandList::CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, StagingImage& src, const ImageSliceSpecification& srcSlice)
    {
        NG_PROFILE("Dx12CommandList::CopyImage()");

        Dx12StagingImage& dxSrc = *api_cast<Dx12StagingImage*>(&src);
        Dx12Image& dxDst = *api_cast<Dx12Image*>(&dst);

        ImageSliceSpecification resDstSlice = ResolveImageSlice(dstSlice, dst.GetSpecification());
        ImageSliceSpecification resSrcSlice = ResolveImageSlice(srcSlice, src.GetSpecification());

        UINT dstSubresource = CalculateSubresource(resDstSlice.ImageMipLevel, resDstSlice.ImageArraySlice, 0, dst.GetSpecification().MipLevels, dst.GetSpecification().ArraySize);

        ImageSubresourceSpecification dstSubresourceSpec = ImageSubresourceSpecification(
            resDstSlice.ImageMipLevel, 1,
            resDstSlice.ImageArraySlice, 1
        );

        m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().RequireImageState(*api_cast<const CommandList*>(this), dst, ImageSubresourceSpecification(resDstSlice.ImageMipLevel, 1, resDstSlice.ImageArraySlice, 1), ResourceState::CopyDst);
        m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().RequireBufferState(*api_cast<const CommandList*>(this), *api_cast<Buffer*>(&dxSrc.GetDx12Buffer()), ResourceState::CopySrc);
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
        NG_PROFILE("Dx12CommandList::CopyBuffer()");

        Dx12Buffer& dxDst = *api_cast<Dx12Buffer*>(&dst);
        Dx12Buffer& dxSrc = *api_cast<Dx12Buffer*>(&src);

        m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().RequireBufferState(*api_cast<const CommandList*>(this), dst, ResourceState::CopyDst);
        m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().RequireBufferState(*api_cast<const CommandList*>(this), src, ResourceState::CopySrc);
        CommitBarriers();

        m_CommandList->CopyBufferRegion(dxDst.GetD3D12Resource().Get(), dstOffset, dxSrc.GetD3D12Resource().Get(), srcOffset, size);
    }

    void Dx12CommandList::Dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) const
    {
        NG_PROFILE("Dx12CommandList::Dispatch()");
        m_CommandList->Dispatch(groupsX, groupsY, groupsZ);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Draw methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12CommandList::DrawIndexed(const DrawArguments& args) const
    {
        NG_PROFILE("Dx12CommandList::DrawIndexed()");
        m_CommandList->DrawIndexedInstanced(args.VertexCount, args.InstanceCount, args.StartIndexLocation, args.StartVertexLocation, args.StartInstanceLocation);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Other methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12CommandList::PushConstants(const void* memory, size_t size, size_t srcOffset, size_t dstOffset)
    {
        NG_PROFILE("Dx12CommandList::PushConstants()");

        NG_ASSERT(m_CurrentGraphicsPipeline || m_CurrentComputePipeline, "[Dx12CommandList] Can't push constants if no pipeline is bound.");
        NG_ASSERT((size + dstOffset <= 128), "[Dx12CommandList] Size + dstOffset exceeds max push constants size (128 bytes)");
        NG_ASSERT((dstOffset % 4 == 0), "[Dx12CommandList] DstOffset must be aligned to 4 bytes.");
        NG_ASSERT((size % 4 == 0), "[Dx12CommandList] Size must be aligned to 4 bytes.");

        if (m_CurrentGraphicsPipeline)
        {
            const Dx12GraphicsPipeline& dxPipeline = *api_cast<const Dx12GraphicsPipeline*>(m_CurrentGraphicsPipeline);
            NG_ASSERT((dxPipeline.GetPushConstantsRootIndex().second != Dx12GraphicsPipeline::RootParameterIndices::Invalid), "[Dx12CommandList] Trying to push constants with no root parameter created for constants.");
            m_CommandList->SetGraphicsRoot32BitConstants(dxPipeline.GetPushConstantsRootIndex().second, static_cast<UINT>(size / 4), static_cast<const uint8_t*>(memory) + srcOffset, static_cast<UINT>(dstOffset / 4));
        }
        else
        {
            const Dx12ComputePipeline& dxPipeline = *api_cast<const Dx12ComputePipeline*>(m_CurrentComputePipeline);
            NG_ASSERT((dxPipeline.GetPushConstantsRootIndex().second != Dx12ComputePipeline::RootParameterIndices::Invalid), "[Dx12CommandList] Trying to push constants with no root parameter created for constants.");
            m_CommandList->SetComputeRoot32BitConstants(dxPipeline.GetPushConstantsRootIndex().second, static_cast<UINT>(size / 4), static_cast<const uint8_t*>(memory) + srcOffset, static_cast<UINT>(dstOffset / 4));
        }
    }

}