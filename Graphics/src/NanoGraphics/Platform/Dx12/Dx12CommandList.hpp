#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/SwapchainSpec.hpp"
#include "NanoGraphics/Renderer/CommandListSpec.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"

#include <utility>

namespace Nano::Graphics
{
	class Swapchain;
	class Image;
	class StagingImage;
	class Buffer;
	class Renderpass;
	class CommandList;
	class CommandListPool;
}

namespace Nano::Graphics::Internal
{

	class Dx12Swapchain;
	class Dx12CommandList;
	class Dx12CommandListPool;

#if defined(NG_API_DX12)
	////////////////////////////////////////////////////////////////////////////////////
	// Dx12CommandListPool
	////////////////////////////////////////////////////////////////////////////////////
	class Dx12CommandListPool
	{
	public:
		// Constructor & Destructor
		Dx12CommandListPool(Swapchain& swapchain, const CommandListPoolSpecification& specs);
		~Dx12CommandListPool();

		// Methods
		void FreeList(CommandList& list) const;
		void FreeLists(std::span<CommandList*> lists) const;

		void Reset() const;

		// Getters
		inline const CommandListPoolSpecification& GetSpecification() const { return m_Specification; }

		// Internal getters
		inline Dx12Swapchain& GetDx12Swapchain() const { return m_Swapchain; }

		inline DxPtr<ID3D12CommandAllocator> GetD3D12CommandAllocator() const { return m_CommandAllocator; }

	private:
		Dx12Swapchain& m_Swapchain;
		CommandListPoolSpecification m_Specification;

		DxPtr<ID3D12CommandAllocator> m_CommandAllocator = nullptr;

		friend class Dx12Swapchain;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// Dx12CommandList
	////////////////////////////////////////////////////////////////////////////////////
	class Dx12CommandList
	{
	public:
		Dx12CommandList(CommandListPool& pool, const CommandListSpecification& specs);
		~Dx12CommandList();

		// Methods
		void Open();
		void Close();

		void Submit(const CommandListSubmitArgs& args);

		void WaitTillComplete() const;

		void CommitBarriers();

		// Object methods
		void StartRenderpass(const RenderpassStartArgs& args);
		void EndRenderpass(const RenderpassEndArgs& args);

		void BindPipeline(const GraphicsPipeline& pipeline);
		void BindPipeline(const ComputePipeline& pipeline);

		void BindBindingSet(const BindingSet& set, std::span<const uint32_t> dynamicOffsets);
		void BindBindingSets(std::span<const BindingSet*> sets, std::span<const std::span<const uint32_t>> dynamicOffsets);

		void SetViewport(const Viewport& viewport) const;
		void SetScissor(const ScissorRect& scissor) const;

		void BindVertexBuffer(const Buffer& buffer) const;
		void BindIndexBuffer(const Buffer& buffer) const;

		void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, Image& src, const ImageSliceSpecification& srcSlice);
		void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, StagingImage& src, const ImageSliceSpecification& srcSlice);
		void CopyBuffer(Buffer& dst, Buffer& src, size_t size, size_t srcOffset, size_t dstOffset);

		void Dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) const;

		// Draw methods
		void DrawIndexed(const DrawArguments& args) const;

		// Other methods
		void PushConstants(const void* memory, size_t size, size_t srcOffset, size_t dstOffset);

		// Getters
		inline const CommandListSpecification& GetSpecification() const { return m_Specification; }

		// Internal Getters
		inline DxPtr<ID3D12GraphicsCommandList10> GetID3D12GraphicsCommandList() const { return m_CommandList; }
		inline HANDLE GetWaitIdleEvent() const { return m_WaitIdleEvent; }

	private:
		Dx12CommandListPool& m_Pool;
		CommandListSpecification m_Specification;

		DxPtr<ID3D12GraphicsCommandList10> m_CommandList = nullptr;

		const GraphicsPipeline* m_CurrentGraphicsPipeline = nullptr;
		const ComputePipeline* m_CurrentComputePipeline = nullptr;

		uint64_t m_SignaledValue = 0;
		HANDLE m_WaitIdleEvent = nullptr;

		friend class Dx12CommandListPool;
	};
#endif

}