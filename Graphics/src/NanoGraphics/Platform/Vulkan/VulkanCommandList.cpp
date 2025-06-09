#include "ngpch.h"
#include "VulkanCommandList.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/CommandList.hpp"
#include "NanoGraphics/Renderer/Swapchain.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"

namespace Nano::Graphics::Internal
{

    static_assert(std::is_same_v<Device::Type, VulkanDevice>, "Current Device::Type is not VulkanDevice and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<Swapchain::Type, VulkanSwapchain>, "Current Swapchain::Type is not VulkanSwapchain and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<CommandList::Type, VulkanCommandList>, "Current CommandList::Type is not VulkanCommandList and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<CommandListPool::Type, VulkanCommandListPool>, "Current CommandListPool::Type is not VulkanImage and Vulkan source code is being compiled.");

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanCommandListPool::VulkanCommandListPool(const Swapchain& swapchain, const CommandListPoolSpecification& specs)
        : m_ExecutionRegion(*reinterpret_cast<const VulkanSwapchain*>(&swapchain)), m_Specification(specs)
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Note: Allows us to reset the command buffer and reuse it.
        poolInfo.queueFamilyIndex = m_ExecutionRegion.GetVulkanDevice().GetContext().GetVulkanPhysicalDevice().GetQueueFamilyIndices().QueueFamily;
        
        VK_VERIFY(vkCreateCommandPool(m_ExecutionRegion.GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkDevice(), &poolInfo, VulkanAllocator::GetCallbacks(), &m_CommandPool));

        m_ExecutionRegion.GetVulkanDevice().GetContext().SetDebugName(m_CommandPool, VK_OBJECT_TYPE_COMMAND_POOL, std::string(specs.DebugName));
    }

    VulkanCommandListPool::~VulkanCommandListPool()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanCommandListPool::FreeList(CommandList& list) const
    {
        const VulkanContext& context = m_ExecutionRegion.GetVulkanDevice().GetContext();

        VkDevice device = context.GetVulkanLogicalDevice().GetVkDevice();
        VkCommandBuffer commandBuffer = (*reinterpret_cast<VulkanCommandList*>(&list)).GetVkCommandBuffer();
        m_ExecutionRegion.GetVulkanDevice().GetContext().Destroy([device, commandPool = m_CommandPool, commandBuffer]() mutable
        { 
            vkFreeCommandBuffers(device, commandPool, 1ul, &commandBuffer);
        });
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

        VkDevice device = m_ExecutionRegion.GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkDevice();
        m_ExecutionRegion.GetVulkanDevice().GetContext().Destroy([device, commandPool = m_CommandPool, commandBuffers = std::move(commandBuffers)]() mutable
        {
            vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        });
    }

    void VulkanCommandListPool::ResetList(CommandList& list) const
    {
        vkResetCommandBuffer((*reinterpret_cast<VulkanCommandList*>(&list)).GetVkCommandBuffer(), 0);
    }

    void VulkanCommandListPool::ResetAll() const
    {
        vkResetCommandPool(m_ExecutionRegion.GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkDevice(), m_CommandPool, 0);
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

        VK_VERIFY(vkAllocateCommandBuffers(m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkDevice(), &allocInfo, &m_CommandBuffer));

        m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetContext().SetDebugName(m_CommandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, std::string(specs.DebugName));
    }

    VulkanCommandList::~VulkanCommandList()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanCommandList::Reset() const
    {
        vkResetCommandBuffer(m_CommandBuffer, 0);
    }

    void VulkanCommandList::ResetAndOpen()
    {
        Reset();
        Open();
    }

    void VulkanCommandList::Open()
    {
        m_WaitStage = VK_PIPELINE_STAGE_2_NONE;

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        {
            NG_PROFILE("VulkanCommandList::Open::Begin");
            VK_VERIFY(vkBeginCommandBuffer(m_CommandBuffer, &beginInfo));
        }
    }

    void VulkanCommandList::Close() const
    {
        NG_PROFILE("VulkanCommandList::Close()");
        VK_VERIFY(vkEndCommandBuffer(m_CommandBuffer));
    }

    void VulkanCommandList::Submit(const CommandListSubmitArgs& args) const 
    {
        NG_PROFILE("VulkanCommandBuffer::Submit()");

        VkSemaphoreSubmitInfo waitInfo1 = {};
        waitInfo1.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        waitInfo1.semaphore = nullptr;
        waitInfo1.stageMask = m_WaitStage;
        waitInfo1.value = 0;

        VkSemaphoreSubmitInfo signalInfo = {};
        waitInfo1.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        waitInfo1.semaphore = nullptr;
        waitInfo1.value = 0;

        VkCommandBufferSubmitInfo commandInfo = {};
        commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandInfo.commandBuffer = m_CommandBuffer;

        VkSubmitInfo2 submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &commandInfo;
        
        VK_VERIFY(vkQueueSubmit2(m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkQueue(args.Queue), 1, &submitInfo, nullptr));
    }

}