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
    static_assert(std::is_same_v<CommandList::Type, VulkanCommandList>, "Current CommandList::Type is not VulkanCommandList and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<CommandListPool::Type, VulkanCommandListPool>, "Current CommandListPool::Type is not VulkanImage and Vulkan source code is being compiled.");

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanCommandListPool::VulkanCommandListPool(const Device& device, const CommandListPoolSpecification& specs)
        : m_Device(*reinterpret_cast<const VulkanDevice*>(&device)), m_Specification(specs)
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Note: Allows us to reset the command buffer and reuse it.

        switch (specs.Queue)
        {
        case CommandQueue::Graphics:    poolInfo.queueFamilyIndex = m_Device.GetContext().GetVulkanPhysicalDevice().GetQueueFamilyIndices().GraphicsQueue; break;
        case CommandQueue::Compute:     poolInfo.queueFamilyIndex = m_Device.GetContext().GetVulkanPhysicalDevice().GetQueueFamilyIndices().ComputeQueue; break;
        case CommandQueue::Present:     poolInfo.queueFamilyIndex = m_Device.GetContext().GetVulkanPhysicalDevice().GetQueueFamilyIndices().PresentQueue; break;

        default:
            NG_UNREACHABLE();
        }
        
        VK_VERIFY(vkCreateCommandPool(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &poolInfo, m_Device.GetAllocator().GetCallbacks(), &m_CommandPool));

        m_Device.GetContext().SetDebugName(m_CommandPool, VK_OBJECT_TYPE_COMMAND_POOL, specs.DebugName);
    }

    VulkanCommandListPool::~VulkanCommandListPool()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanCommandListPool::FreeList(CommandList& list) const
    {
        VkCommandBuffer commandBuffer = (*reinterpret_cast<VulkanCommandList*>(&list)).GetVkCommandBuffer();
        vkFreeCommandBuffers(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), m_CommandPool, 1ul, &commandBuffer);
    }

    void VulkanCommandListPool::FreeLists(std::span<CommandList*> lists) const
    {
        std::vector<VkCommandBuffer> commandBuffers;
        commandBuffers.reserve(lists.size());

        for (auto& list : lists)
        {
            VkCommandBuffer commandBuffer = (*reinterpret_cast<VulkanCommandList*>(&list)).GetVkCommandBuffer();
            commandBuffers.push_back(commandBuffer);
        }

        vkFreeCommandBuffers(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), m_CommandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    }

    void VulkanCommandListPool::ResetList(CommandList& list) const
    {
        vkResetCommandBuffer((*reinterpret_cast<VulkanCommandList*>(&list)).GetVkCommandBuffer(), 0);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanCommandList::VulkanCommandList(const CommandListPool& pool, const CommandListSpecification& specs)
        : m_Pool(*reinterpret_cast<const VulkanCommandListPool*>(&pool)), m_Specification(specs)
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_Pool.GetVkCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VK_VERIFY(vkAllocateCommandBuffers(m_Pool.GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkDevice(), &allocInfo, &m_CommandBuffer));

        m_Pool.GetVulkanDevice().GetContext().SetDebugName(m_CommandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, specs.DebugName);
    }

    VulkanCommandList::~VulkanCommandList()
    {
    }

}