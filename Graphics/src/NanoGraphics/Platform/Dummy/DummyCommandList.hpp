#pragma once

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ShaderSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"
#include "NanoGraphics/Renderer/CommandListSpec.hpp"

#include <array>
#include <unordered_map>

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

	class DummySwapchain;
	class DummyCommandList;
	class DummyCommandListPool;

#if 1 //defined(NG_API_DUMMY)
	////////////////////////////////////////////////////////////////////////////////////
	// DummyCommandListPool
	////////////////////////////////////////////////////////////////////////////////////
	class DummyCommandListPool : public Traits::NoCopy
	{
	public:
		// Constructor & Destructor
		inline constexpr DummyCommandListPool(Swapchain& swapchain, const CommandListPoolSpecification& specs)
			: m_Specification(specs) { (void)swapchain; }
		constexpr ~DummyCommandListPool() = default;

		// Methods
		inline constexpr void FreeList(CommandList& list) const { (void)list; }
		inline constexpr void FreeLists(std::span<CommandList*> lists) const { (void)lists; }

		inline constexpr void ResetList(CommandList& list) const { (void)list; }
		inline constexpr void ResetAll() const {}

		// Getters
		inline constexpr const CommandListPoolSpecification& GetSpecification() const { return m_Specification; }

	private:
		CommandListPoolSpecification m_Specification;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// DummyCommandList
	////////////////////////////////////////////////////////////////////////////////////
	class DummyCommandList : public Traits::NoCopy
	{
	public:
		// Constructor & Destructor
		inline constexpr DummyCommandList(CommandListPool& pool, const CommandListSpecification& specs)
			: m_Specification(specs) { (void)pool; }
		inline constexpr ~DummyCommandList() = default;

		// Methods
		inline constexpr void Reset() const {}

		inline constexpr void ResetAndOpen() {}
		inline constexpr void Open() {}
		inline constexpr void Close() {}

		inline constexpr void Submit(const CommandListSubmitArgs& args) const { (void)args; }

		inline constexpr void WaitTillComplete() const {}

		inline constexpr void CommitBarriers() {}

		// Object methods
		inline constexpr void SetGraphicsState(const GraphicsState& state) { (void)state; }

		inline constexpr void SetViewport(const Viewport& viewport) const { (void)viewport; }
		inline constexpr void SetScissor(const ScissorRect& scissor) const { (void)scissor; }

		inline constexpr void BindVertexBuffer(const Buffer& buffer) const { (void)buffer; }
		inline constexpr void BindIndexBuffer(const Buffer& buffer) const { (void)buffer; }

		inline constexpr void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, Image& src, const ImageSliceSpecification& srcSlice) { (void)dst; (void)dstSlice; (void)src; (void)srcSlice; }
		inline constexpr void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, StagingImage& src, const ImageSliceSpecification& srcSlice) { (void)dst; (void)dstSlice; (void)src; (void)srcSlice; }
		inline constexpr void CopyBuffer(Buffer& dst, Buffer& src, size_t size, size_t srcOffset, size_t dstOffset) { (void)dst; (void)src; (void)size; (void)srcOffset; (void)dstOffset; }

		// Draw methods
		inline constexpr void DrawIndexed(const DrawArguments& args) const { (void)args; }

		// Getters
		inline constexpr const CommandListSpecification& GetSpecification() const { return m_Specification; }

	private:
		CommandListSpecification m_Specification;
	};
#endif

}