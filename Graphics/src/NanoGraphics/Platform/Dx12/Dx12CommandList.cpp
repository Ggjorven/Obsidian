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

#include "NanoGraphics/Platform/Dx12/Dx12Device.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Buffer.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Image.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Resources.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Renderpass.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Framebuffer.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Pipeline.hpp"

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

        m_Swapchain.GetDx12Device().GetContext().Destroy([commandList = dxCommandList.GetID3D12GraphicsCommandList()]() {}); // Note: Holding a reference to the resource is enough to keep it alive (and destroy when the scope ends)

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
    }

    void Dx12CommandList::Submit(const CommandListSubmitArgs& args) const
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
            DX_VERIFY(queue->Wait(m_Pool.GetDx12Swapchain().GetD3D12Fence().Get(), m_Pool.GetDx12Swapchain().GetPreviousCommandListWaitValue(*api_cast<const Dx12CommandList*>(&list))));
        }
        
        ID3D12CommandList* lists[] = { m_CommandList.Get() };
        queue->ExecuteCommandLists(1, lists);

        uint64_t signalValue = m_Pool.GetDx12Swapchain().RetrieveCommandListWaitValue(*this);
        DX_VERIFY(queue->Signal(m_Pool.GetDx12Swapchain().GetD3D12Fence().Get(), signalValue));
        
        if (args.OnFinishMakeSwapchainPresentable)
            m_Pool.GetDx12Swapchain().SetPresentableValue(signalValue);
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

    }

    void Dx12CommandList::BindPipeline(const ComputePipeline& pipeline)
    {
        NG_PROFILE("Dx12CommandList::BindPipeline()");

    }

    void Dx12CommandList::BindBindingSet(const GraphicsPipeline& pipeline, const BindingSet& set)
    {
        NG_PROFILE("Dx12CommandList::BindBindingSet()");

    }

    void Dx12CommandList::BindBindingSets(const GraphicsPipeline& pipeline, std::span<const BindingSet*> sets)
    {
        NG_PROFILE("Dx12CommandList::BindBindingSets()");

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
    }

    void Dx12CommandList::BindIndexBuffer(const Buffer& buffer) const
    {
        NG_PROFILE("Dx12CommandList::BindIndexBuffer()");
    }

    void Dx12CommandList::CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, Image& src, const ImageSliceSpecification& srcSlice)
    {
        NG_PROFILE("Dx12CommandList::CopyImage()");
    }

    void Dx12CommandList::CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, StagingImage& src, const ImageSliceSpecification& srcSlice)
    {
        NG_PROFILE("Dx12CommandList::CopyImage()");
    }

    void Dx12CommandList::CopyBuffer(Buffer& dst, Buffer& src, size_t size, size_t srcOffset, size_t dstOffset)
    {
        NG_PROFILE("Dx12CommandList::CopyBuffer()");
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
    }

}