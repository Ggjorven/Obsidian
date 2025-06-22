#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/SwapchainSpec.hpp"
#include "NanoGraphics/Renderer/Image.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"

#include <utility>

namespace Nano::Graphics
{
	class Swapchain;
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

		inline ID3D12CommandAllocator* GetD3D12CommandAllocator() const { return m_CommandAllocator; }

	private:
		Dx12Swapchain& m_Swapchain;
		CommandListPoolSpecification m_Specification;

		ID3D12CommandAllocator* m_CommandAllocator = nullptr;
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

		void Submit(const CommandListSubmitArgs& args) const;

		void WaitTillComplete() const;

		void CommitBarriers();

		// Object methods
		void SetGraphicsState(const GraphicsState& state);
		void SetComputeState(const ComputeState& state);

		void Dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) const;

		void SetViewport(const Viewport& viewport) const;
		void SetScissor(const ScissorRect& scissor) const;

		void BindVertexBuffer(const Buffer& buffer) const;
		void BindIndexBuffer(const Buffer& buffer) const;

		void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, Image& src, const ImageSliceSpecification& srcSlice);
		void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, StagingImage& src, const ImageSliceSpecification& srcSlice);
		void CopyBuffer(Buffer& dst, Buffer& src, size_t size, size_t srcOffset, size_t dstOffset);

		// Draw methods
		void DrawIndexed(const DrawArguments& args) const;

		// Getters
		inline const CommandListSpecification& GetSpecification() const { return m_Specification; }

		// Internal Getters
		inline ID3D12GraphicsCommandList* GetID3D12GraphicsCommandList() const { return m_CommandList; }

	private:
		Dx12CommandListPool& m_Pool;
		CommandListSpecification m_Specification;

		ID3D12GraphicsCommandList* m_CommandList = nullptr;

		GraphicsState m_GraphicsState = {};
		ComputeState m_ComputeState = {};
	};
#endif

}