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

        // End renderpass
        if (m_GraphicsState.Pass)
        {
            NG_PROFILE("Dx12CommandList::Close::Renderpass");
            m_CommandList->EndRenderPass();

            // Transition to FinalState
            {
                Dx12Renderpass& dxRenderpass = *api_cast<Dx12Renderpass*>(m_GraphicsState.Pass);
                Dx12Framebuffer& dxFramebuffer = *api_cast<Dx12Framebuffer*>(m_GraphicsState.Frame);

                if (dxFramebuffer.GetSpecification().ColourAttachment.IsValid())
                    m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().RequireImageState(*api_cast<const CommandList*>(this), *dxFramebuffer.GetSpecification().ColourAttachment.ImagePtr, dxFramebuffer.GetSpecification().ColourAttachment.Subresources, dxRenderpass.GetSpecification().ColourImageEndState);
                if (dxFramebuffer.GetSpecification().DepthAttachment.IsValid())
                    m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().RequireImageState(*api_cast<const CommandList*>(this), *dxFramebuffer.GetSpecification().DepthAttachment.ImagePtr, dxFramebuffer.GetSpecification().DepthAttachment.Subresources, dxRenderpass.GetSpecification().DepthImageEndState);
                CommitBarriers();
            }
        }

        m_GraphicsState = GraphicsState();
        m_ComputeState = ComputeState();

        // Close commandlist
        {
            NG_PROFILE("Dx12CommandList::Close::CommandList");
            DX_VERIFY(m_CommandList->Close());
        }
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
    void Dx12CommandList::SetGraphicsState(const GraphicsState& state)
    {
        NG_PROFILE("Dx12CommandList::SetGraphicsState()");
        m_GraphicsState = state;

        //NG_ASSERT(m_GraphicsState.Pipeline, "[Dx12CommandList] No pipeline passed in.");
        NG_ASSERT(m_GraphicsState.Pass, "[Dx12CommandList] No Renderpass passed in.");

        // Renderpass
        {
            NG_PROFILE("Dx12CommandList::SetGraphicsState::Renderpass");

            Dx12Renderpass& renderpass = *api_cast<Dx12Renderpass*>(m_GraphicsState.Pass);
            if (!m_GraphicsState.Frame)
            {
                NG_ASSERT((renderpass.GetFramebuffers().size() == m_Pool.GetDx12Swapchain().GetImageCount()), "[Dx12CommandList] No framebuffer was passed into GraphicsState, but renderpass' framebuffer count doesn't align with swapchain image count.");
                m_GraphicsState.Frame = &renderpass.GetFramebuffer(static_cast<uint8_t>(m_Pool.GetDx12Swapchain().GetAcquiredImage()));
            }
            Dx12Framebuffer& framebuffer = *api_cast<Dx12Framebuffer*>(m_GraphicsState.Frame);
            
            Dx12Image* colourImage = api_cast<Dx12Image*>(framebuffer.GetSpecification().ColourAttachment.ImagePtr);
            Dx12Image* depthImage = api_cast<Dx12Image*>(framebuffer.GetSpecification().DepthAttachment.ImagePtr);

            // Validation checks
            if constexpr (Information::Validation)
            {
                if (colourImage)
                {
                    ResourceState currentState = m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().GetResourceState(*api_cast<Image*>(colourImage), framebuffer.GetSpecification().ColourAttachment.Subresources);
                    ResourceState startState = renderpass.GetSpecification().ColourImageStartState;
                    if (currentState != startState)
                        m_Pool.GetDx12Swapchain().GetDx12Device().GetContext().Error(std::format("[Dx12CommandList] Current colour image state ({0}) doesn't match the renderpass' specified colour image start state ({1}).", ResourceStateToString(currentState), ResourceStateToString(startState)));
                }
                if (depthImage)
                {
                    ResourceState currentState = m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().GetResourceState(*api_cast<Image*>(depthImage), framebuffer.GetSpecification().DepthAttachment.Subresources);
                    ResourceState startState = renderpass.GetSpecification().DepthImageStartState;
                    if (currentState != startState)
                        m_Pool.GetDx12Swapchain().GetDx12Device().GetContext().Error(std::format("[Dx12CommandList] Current depth image state ({0}) doesn't match the renderpass' specified depth image start state ({1}).", ResourceStateToString(currentState), ResourceStateToString(startState)));
                }
            }

            D3D12_RENDER_PASS_RENDER_TARGET_DESC colourDesc = {};
            D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthDesc = {};

            if (colourImage)
            {
                colourDesc.cpuDescriptor = colourImage->GetSubresourceView(framebuffer.GetSpecification().ColourAttachment.Subresources, ImageSubresourceViewUsage::RTV).GetCPUHandle();
                colourDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR; // TODO: ...
                colourDesc.BeginningAccess.Clear.ClearValue.Format = FormatToFormatMapping(colourImage->GetSpecification().ImageFormat).RTVFormat;
                colourDesc.BeginningAccess.Clear.ClearValue.Color[0] = m_GraphicsState.ColourClear.r;
                colourDesc.BeginningAccess.Clear.ClearValue.Color[1] = m_GraphicsState.ColourClear.g;
                colourDesc.BeginningAccess.Clear.ClearValue.Color[2] = m_GraphicsState.ColourClear.b;
                colourDesc.BeginningAccess.Clear.ClearValue.Color[3] = m_GraphicsState.ColourClear.a;
                
                colourDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE; // TODO: ...

                // Transition to rendering state
                if (renderpass.GetSpecification().ColourImageStartState != renderpass.GetSpecification().ColourImageRenderingState)
                    m_Pool.GetDx12Swapchain().GetDx12Device().GetTracker().RequireImageState(*api_cast<const CommandList*>(this), *api_cast<Image*>(colourImage), framebuffer.GetSpecification().ColourAttachment.Subresources, renderpass.GetSpecification().ColourImageRenderingState);
            }
            if (depthImage)
            {
                // TODO: ...
            }
            CommitBarriers();

            // TODO: Flags
            {
                NG_PROFILE("Dx12CommandList::SetGraphicsState::BeginRenderpass");
                m_CommandList->BeginRenderPass((colourImage ? 1 : 0), (colourImage ? &colourDesc : nullptr), (depthImage ? &depthDesc : nullptr), D3D12_RENDER_PASS_FLAG_NONE);
            }
        }

        // Pipeline
        {

        }
    }

    void Dx12CommandList::SetComputeState(const ComputeState& state)
    {
        NG_PROFILE("Dx12CommandList::SetComputeState()");
    }

    void Dx12CommandList::Dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) const
    {
        NG_PROFILE("Dx12CommandList::Dispatch()");
    }

    void Dx12CommandList::SetViewport(const Viewport& viewport) const
    {
        NG_PROFILE("Dx12CommandList::SetViewport()");
    }

    void Dx12CommandList::SetScissor(const ScissorRect& scissor) const
    {
        NG_PROFILE("Dx12CommandList::SetScissor()");
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

    ////////////////////////////////////////////////////////////////////////////////////
    // Draw methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12CommandList::DrawIndexed(const DrawArguments& args) const
    {
        NG_PROFILE("Dx12CommandList::DrawIndexed()");
    }

}