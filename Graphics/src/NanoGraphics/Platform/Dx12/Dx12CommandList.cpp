#include "ngpch.h"
#include "Dx12CommandList.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Core/Window.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/CommandList.hpp"
#include "NanoGraphics/Renderer/Swapchain.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12Device.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Resources.hpp"

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
        m_CommandAllocator->AddRef();

        if constexpr (Information::Validation)
        {
            if (!m_Specification.DebugName.empty())
                m_Swapchain.GetDx12Device().GetContext().SetDebugName(m_CommandAllocator, std::string(m_Specification.DebugName));
        }
    }

	Dx12CommandListPool::~Dx12CommandListPool()
	{
		m_Swapchain.GetDx12Device().GetContext().Destroy([commandAllocator = m_CommandAllocator]() 
		{
			commandAllocator->Release();
		});
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void Dx12CommandListPool::FreeList(CommandList& list) const
	{
        Dx12CommandList& dxCommandList = *api_cast<Dx12CommandList*>(&list);

        m_Swapchain.GetDx12Device().GetContext().Destroy([commandList = dxCommandList.GetID3D12GraphicsCommandList()]()
        {
            commandList->Release();
        });
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
        DX_VERIFY(m_Pool.GetDx12Swapchain().GetDx12Device().GetContext().GetD3D12Device()->CreateCommandList(0, CommandQueueToD3D12CommandListType(m_Pool.GetSpecification().Queue), m_Pool.GetD3D12CommandAllocator(), nullptr, IID_PPV_ARGS(&m_CommandList)));
        m_CommandList->AddRef();
        DX_VERIFY(m_CommandList->Close());

        if constexpr (Information::Validation)
        {
            if (!m_Specification.DebugName.empty())
                m_Pool.GetDx12Swapchain().GetDx12Device().GetContext().SetDebugName(m_CommandList, std::string(m_Specification.DebugName));
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
        DX_VERIFY(m_CommandList->Reset(m_Pool.GetD3D12CommandAllocator(), nullptr));
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
            DX_VERIFY(queue->Wait(m_Pool.GetDx12Swapchain().GetD3D12Fence(), m_Pool.GetDx12Swapchain().GetPreviousCommandListWaitValue(*api_cast<const Dx12CommandList*>(&list))));
        }
        
        ID3D12CommandList* lists[] = { m_CommandList };
        queue->ExecuteCommandLists(1, lists);

        uint64_t signalValue = m_Pool.GetDx12Swapchain().RetrieveCommandListWaitValue(*this);
        DX_VERIFY(queue->Signal(m_Pool.GetDx12Swapchain().GetD3D12Fence(), signalValue));
        
        if (args.OnFinishMakeSwapchainPresentable)
            m_Pool.GetDx12Swapchain().SetPresentableValue(signalValue);
    }

    void Dx12CommandList::CommitBarriers()
    {
        NG_PROFILE("Dx12CommandList::CommitBarriers()");
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Object methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12CommandList::SetGraphicsState(const GraphicsState& state)
    {
        NG_PROFILE("Dx12CommandList::SetGraphicsState()");
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