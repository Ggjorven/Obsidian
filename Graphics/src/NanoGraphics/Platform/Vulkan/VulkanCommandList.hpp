#pragma once

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/CommandListSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <type_traits>

namespace Nano::Graphics
{
	class Device;
	class CommandListPool;
}

namespace Nano::Graphics::Internal
{

	class VulkanDevice;

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanCommandListPool
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanCommandListPool
	{
	public:
		// Constructor & Destructor
		VulkanCommandListPool(const Device& device);
		~VulkanCommandListPool();

	private:
		const VulkanDevice& m_Device;

		VkCommandPool m_CommandPool = VK_NULL_HANDLE;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanCommandList
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanCommandList
	{
	public:
		// Constructor & Destructor
		VulkanCommandList(const CommandListPool& pool, const CommandListSpecification& specs);
		~VulkanCommandList();

	private:
		const VulkanCommandListPool& m_Pool;
		CommandListSpecification m_Specification;

		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
	};

}