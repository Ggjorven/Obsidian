#include "ngpch.h"
#include "VulkanQueues.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanQueues::VulkanQueues(const VulkanContext& context)
        : m_Context(context)
    {	
        const QueueFamilyIndices& indices = m_Context.GetVulkanPhysicalDevice().GetQueueFamilyIndices();
        
        // Retrieve the graphics/compute/present queue handle
        VkDevice vkDevice = m_Context.GetVulkanLogicalDevice().GetVkDevice();
        {
            VkDeviceQueueInfo2 queueInfo = {};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
            queueInfo.queueFamilyIndex = indices.QueueFamily;
            queueInfo.flags = 0; // Note: Should always be 0

            queueInfo.queueIndex = indices.GraphicsQueue;
            vkGetDeviceQueue2(vkDevice, &queueInfo, &m_Queues[static_cast<size_t>(CommandQueue::Graphics)]);

            queueInfo.queueIndex = indices.ComputeQueue;
            vkGetDeviceQueue2(vkDevice, &queueInfo, &m_Queues[static_cast<size_t>(CommandQueue::Compute)]);

            queueInfo.queueIndex = indices.PresentQueue;
            vkGetDeviceQueue2(vkDevice, &queueInfo, &m_Queues[static_cast<size_t>(CommandQueue::Present)]);
        }

        /*
        // Create timeline semaphore
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkSemaphoreTypeCreateInfo timelineInfo = {};
        timelineInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        timelineInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        timelineInfo.initialValue = 0;

        semaphoreInfo.pNext = &timelineInfo;
        VK_VERIFY(vkCreateSemaphore(vkDevice, &semaphoreInfo, nullptr, &m_Timeline));
        */

    }

    VulkanQueues::~VulkanQueues()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    /*
    void VulkanQueues::Submit(CommandQueue queue, VkCommandBuffer cmdBuf) const
    {
        NG_PROFILE("VulkanQueues::Submit()");

        VkCommandBufferSubmitInfo cmdInfo = {};
        cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdInfo.commandBuffer = cmdBuf;

        VkSubmitInfo2 submitInfo = {};
        submitInfo.commandBufferInfoCount = 1;
        submitInfo.pCommandBufferInfos = &cmdInfo;

        VK_VERIFY(vkQueueSubmit2(GetQueue(queue), 1, &submitInfo, VK_NULL_HANDLE));
    }
    */

}