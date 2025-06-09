#pragma once

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/CommandListSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <type_traits>

namespace Nano::Graphics
{
	class ExecutionRegion;
	class CommandList;
	class CommandListPool;
}

namespace Nano::Graphics::Internal
{

	class VulkanDevice;
	class VulkanExecutionRegion;

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanCommandListPool
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanCommandListPool : public Traits::NoCopy
	{
	public:
		// Constructor & Destructor
		VulkanCommandListPool(const ExecutionRegion& execRegion, const CommandListPoolSpecification& specs);
		~VulkanCommandListPool();

		// Methods
		void FreeList(CommandList& list) const;
		void FreeLists(std::span<CommandList*> lists) const;

		void ResetList(CommandList& list) const;
		void ResetAll() const;

		// Getters
		inline const CommandListPoolSpecification& GetSpecification() const { return m_Specification; }

		// Internal Getters
		inline const VulkanExecutionRegion& GetVulkanExecutionRegion() const { return m_ExecutionRegion; }

		inline VkCommandPool GetVkCommandPool() const { return m_CommandPool; }

	private:
		const VulkanExecutionRegion& m_ExecutionRegion;
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
		VulkanCommandList(const CommandListPool& pool, const CommandListSpecification& specs);
		~VulkanCommandList();

		// Methods
		void Begin(bool reset) const;
		void End() const;
		void Submit(const CommandListSubmitArgs& args) const;

		// Getters
		inline const CommandListSpecification& GetSpecification() const { return m_Specification; }

		// Internal Getters
		inline VkCommandBuffer GetVkCommandBuffer() const { return m_CommandBuffer; }

	private:
		const VulkanCommandListPool& m_Pool;
		CommandListSpecification m_Specification;

		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
	};

}