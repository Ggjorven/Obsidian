#pragma once

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ShaderSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"
#include "NanoGraphics/Renderer/CommandListSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <array>

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

	class VulkanDevice;
	class VulkanSwapchain;
	class VulkanCommandList;
	class VulkanCommandListPool;

#if defined(NG_API_VULKAN)
	////////////////////////////////////////////////////////////////////////////////////
	// VulkanCommandListPool
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanCommandListPool
	{
	public:
		// Constructor & Destructor
		VulkanCommandListPool(Swapchain& swapchain, const CommandListPoolSpecification& specs);
		~VulkanCommandListPool();

		// Methods
		void FreeList(CommandList& list) const;
		void FreeLists(std::span<CommandList*> lists) const;

		void Reset() const;

		// Getters
		inline const CommandListPoolSpecification& GetSpecification() const { return m_Specification; }

		// Internal Getters
		inline VulkanSwapchain& GetVulkanSwapchain() { return m_Swapchain; }
		inline const VulkanSwapchain& GetVulkanSwapchain() const { return m_Swapchain; }

		inline VkCommandPool GetVkCommandPool() const { return m_CommandPool; }

	private:
		VulkanSwapchain& m_Swapchain;
		CommandListPoolSpecification m_Specification;

		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanCommandList
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanCommandList
	{
	public:
		// Constructor & Destructor
		VulkanCommandList(CommandListPool& pool, const CommandListSpecification& specs);
		~VulkanCommandList();

		// Methods
		void Open();
		void Close();

		void Submit(const CommandListSubmitArgs& args) const;

		void WaitTillComplete() const;

		void CommitBarriers();

		// Object methods
		void StartRenderpass(const RenderpassStartArgs& args);
		void EndRenderpass(const RenderpassEndArgs& args);
		
		void SetViewport(const Viewport& viewport) const;
		void SetScissor(const ScissorRect& scissor) const;

		void BindPipeline(const GraphicsPipeline& pipeline);
		void BindPipeline(const ComputePipeline& pipeline);
		
		void BindBindingSet(const BindingSet& set);
		void BindBindingSets(const std::span<const BindingSet*> sets);

		void BindVertexBuffer(const Buffer& buffer) const;
		void BindIndexBuffer(const Buffer& buffer) const;

		void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, Image& src, const ImageSliceSpecification& srcSlice);
		void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, StagingImage& src, const ImageSliceSpecification& srcSlice);
		void CopyBuffer(Buffer& dst, Buffer& src, size_t size, size_t srcOffset, size_t dstOffset);

		void Dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) const;

		// Draw methods
		void DrawIndexed(const DrawArguments& args) const;

		// Getters
		inline const CommandListSpecification& GetSpecification() const { return m_Specification; }

		// Internal Getters
		inline VkCommandBuffer GetVkCommandBuffer() const { return m_CommandBuffer; }

	private:
		// Private methods
		void SetWaitStage(VkPipelineStageFlags2 waitStage);

	private:
		VulkanCommandListPool& m_Pool;
		CommandListSpecification m_Specification;

		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
		VkPipelineStageFlags2 m_WaitStage = VK_PIPELINE_STAGE_2_NONE;

		const GraphicsPipeline* m_CurrentGraphicsPipeline = nullptr;
	};
#endif

}