#pragma once

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/CommandListSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <type_traits>

namespace Nano::Graphics
{
	class Swapchain;
	class CommandList;
	class CommandListPool;
}

namespace Nano::Graphics::Internal
{

	class VulkanDevice;
	class VulkanSwapchain;

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanCommandListPool
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanCommandListPool : public Traits::NoCopy
	{
	public:
		// Constructor & Destructor
		VulkanCommandListPool(Swapchain& swapchain, const CommandListPoolSpecification& specs);
		~VulkanCommandListPool();

		// Methods
		void FreeList(CommandList& list) const;
		void FreeLists(std::span<CommandList*> lists) const;

		void ResetList(CommandList& list) const;
		void ResetAll() const;

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
	class VulkanCommandList : public Traits::NoCopy
	{
	public:
		// Constructor & Destructor
		VulkanCommandList(CommandListPool& pool, const CommandListSpecification& specs);
		~VulkanCommandList();

		// Methods
		void Reset() const;

		void ResetAndOpen();
		void Open();
		void Close() const;

		void Submit(const CommandListSubmitArgs& args) const;

		// Getters
		inline const CommandListSpecification& GetSpecification() const { return m_Specification; }

		// Internal Getters
		inline VkCommandBuffer GetVkCommandBuffer() const { return m_CommandBuffer; }

	private:
		VulkanCommandListPool& m_Pool;
		CommandListSpecification m_Specification;

		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
		VkPipelineStageFlags2 m_WaitStage = VK_PIPELINE_STAGE_2_NONE;
	};

}