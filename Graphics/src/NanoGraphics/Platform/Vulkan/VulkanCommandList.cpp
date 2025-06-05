#include "ngpch.h"
#include "VulkanCommandList.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/CommandList.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"

namespace Nano::Graphics::Internal
{

    static_assert(std::is_same_v<Device::Type, VulkanDevice>, "Current Device::Type is not VulkanDevice and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<CommandListPool::Type, VulkanCommandListPool>, "Current CommandListPool::VulkanCommandListPool is not VulkanImage and Vulkan source code is being compiled.");

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanCommandListPool::VulkanCommandListPool(const Device& device)
        : m_Device(*reinterpret_cast<const VulkanDevice*>(&device))
    {
        //QueueFamilyIndices queueFamilyIndices = QueueFamilyIndices::Find(VulkanContext::GetVulkanPhysicalDevice().GetVkPhysicalDevice());
        //
        //VkCommandPoolCreateInfo poolInfo = {};
        //poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        //poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Allows us to reset the command buffer and reuse it.
        //poolInfo.queueFamilyIndex = queueFamilyIndices.QueueFamily;
        //
        //VK_VERIFY(vkCreateCommandPool(VulkanContext::GetVulkanDevice().GetVkDevice(), &poolInfo, nullptr, &m_CommandPool));
    }

    VulkanCommandListPool::~VulkanCommandListPool()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanCommandList::VulkanCommandList(const CommandListPool& pool, const CommandListSpecification& specs)
        : m_Pool(*reinterpret_cast<const VulkanCommandListPool*>(&pool)), m_Specification(specs)
    {
    }

    VulkanCommandList::~VulkanCommandList()
    {
    }

}