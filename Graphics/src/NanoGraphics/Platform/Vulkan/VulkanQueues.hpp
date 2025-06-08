#pragma once

#include "NanoGraphics/Renderer/CommandListSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics::Internal
{

    class VulkanContext;

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanQueues
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanQueues : public Traits::NoMove
    {
    public:
        // Constructors & Destructor
        VulkanQueues(const VulkanContext& context);
        ~VulkanQueues();

        // Methods

        // Getters
        inline VkQueue GetQueue(CommandQueue queue) const { return m_Queues[static_cast<size_t>(queue)]; }

        // Internal Getters

    private:
        const VulkanContext& m_Context;

        std::array<VkQueue, static_cast<size_t>(CommandQueue::Count)> m_Queues = { };

        VkSemaphore m_Timeline = VK_NULL_HANDLE;
    };

}