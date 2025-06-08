#pragma once

#include "NanoGraphics/Renderer/CommandListSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <Nano/Nano.hpp>

#include <array>
#include <vector>
#include <unordered_map>

namespace Nano::Graphics::Internal
{

    class VulkanContext;

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanQueues
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanQueues : public Traits::NoMove, public Traits::NoCopy
    {
    public:
        // Constructors & Destructor
        VulkanQueues(const VulkanContext& context);
        ~VulkanQueues();

        // Methods
        //void Submit(CommandQueue queue, VkCommandBuffer cmdBuf) const;

        // Getters
        inline VkQueue GetQueue(CommandQueue queue) const { return m_Queues[static_cast<size_t>(queue)]; }

    private:
        const VulkanContext& m_Context;

        std::array<VkQueue, static_cast<size_t>(CommandQueue::Count)> m_Queues = { };
    };

}