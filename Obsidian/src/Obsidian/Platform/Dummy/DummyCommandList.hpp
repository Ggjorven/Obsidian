#pragma once

#include "Obsidian/Renderer/ResourceSpec.hpp"
#include "Obsidian/Renderer/ShaderSpec.hpp"
#include "Obsidian/Renderer/ImageSpec.hpp"
#include "Obsidian/Renderer/CommandListSpec.hpp"

#include <span>
#include <array>
#include <unordered_map>

namespace Obsidian
{
	class Swapchain;
	class Image;
	class StagingImage;
	class Buffer;
	class Renderpass;
	class CommandList;
	class CommandListPool;
}

namespace Obsidian::Internal
{

	class DummySwapchain;
	class DummyCommandList;
	class DummyCommandListPool;

#if 1 //defined(OB_API_DUMMY)
	////////////////////////////////////////////////////////////////////////////////////
	// DummyCommandListPool
	////////////////////////////////////////////////////////////////////////////////////
	class DummyCommandListPool
	{
	public:
		// Constructor & Destructor
		inline constexpr DummyCommandListPool(Swapchain& swapchain, const CommandListPoolSpecification& specs)
			: m_Specification(specs) { (void)swapchain; }
		constexpr ~DummyCommandListPool() = default;

		// Methods
		inline constexpr void FreeList(CommandList& list) const { (void)list; }
		inline constexpr void FreeLists(std::span<CommandList*> lists) const { (void)lists; }

		inline constexpr void Reset() const {}

		// Getters
		inline constexpr const CommandListPoolSpecification& GetSpecification() const { return m_Specification; }

	private:
		CommandListPoolSpecification m_Specification;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// DummyCommandList
	////////////////////////////////////////////////////////////////////////////////////
	class DummyCommandList
	{
	public:
		// Constructor & Destructor
		inline constexpr DummyCommandList(CommandListPool& pool, const CommandListSpecification& specs)
			: m_Specification(specs) { (void)pool; }
		inline constexpr ~DummyCommandList() = default;

		// Methods
		inline constexpr void Open() {}
		inline constexpr void Close() {}

		inline constexpr void Submit(const CommandListSubmitArgs& args) { (void)args; }

		inline constexpr void WaitTillComplete() const {}

		inline constexpr void CommitBarriers() {}

		// Object methods
		inline constexpr void StartRenderpass(const RenderpassStartArgs& args) { (void)args; }
		inline constexpr void EndRenderpass(const RenderpassEndArgs& args) { (void)args; }

		inline constexpr void BindPipeline(const GraphicsPipeline& pipeline) { (void)pipeline; }
		inline constexpr void BindPipeline(const ComputePipeline& pipeline) { (void)pipeline; }

		inline constexpr void BindBindingSet(const BindingSet& set, std::span<const uint32_t> dynamicOffsets) { (void)set; (void)dynamicOffsets; }
		inline constexpr void BindBindingSets(std::span<const BindingSet*> sets, std::span<const std::span<const uint32_t>> dynamicOffsets) { (void)sets; (void)dynamicOffsets; }

		inline constexpr void SetViewport(const Viewport& viewport) const { (void)viewport; }
		inline constexpr void SetScissor(const ScissorRect& scissor) const { (void)scissor; }

		inline constexpr void BindVertexBuffer(const Buffer& buffer) const { (void)buffer; }
		inline constexpr void BindIndexBuffer(const Buffer& buffer) const { (void)buffer; }

		inline constexpr void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, Image& src, const ImageSliceSpecification& srcSlice) { (void)dst; (void)dstSlice; (void)src; (void)srcSlice; }
		inline constexpr void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, StagingImage& src, const ImageSliceSpecification& srcSlice) { (void)dst; (void)dstSlice; (void)src; (void)srcSlice; }
		inline constexpr void CopyBuffer(Buffer& dst, Buffer& src, size_t size, size_t srcOffset, size_t dstOffset) { (void)dst; (void)src; (void)size; (void)srcOffset; (void)dstOffset; }

		inline constexpr void Dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) const { (void)groupsX; (void)groupsY; (void)groupsZ; }

		// State methods
		inline constexpr void RequireState(Image& image, const ImageSubresourceSpecification& subresources, ResourceState state) { (void)image; (void)subresources; (void)state; }
		inline constexpr void RequireState(Buffer& buffer, ResourceState state) { (void)buffer; (void)state; }

		// Draw methods
		inline constexpr void DrawIndexed(const DrawArguments& args) const { (void)args; }

		// Other methods
		inline constexpr void PushConstants(const void* memory, size_t size, size_t srcOffset, size_t dstOffset) { (void)memory; (void)size; (void)srcOffset; (void)dstOffset; }

		// Getters
		inline constexpr const CommandListSpecification& GetSpecification() const { return m_Specification; }

	private:
		CommandListSpecification m_Specification;
	};
#endif

}