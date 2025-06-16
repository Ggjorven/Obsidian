#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/PipelineSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{
    class Device;
}

namespace Nano::Graphics::Internal
{

    class VulkanDevice;

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanGraphicsPipeline
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanGraphicsPipeline
    {
    public:
        // Constructors & Destructor
        VulkanGraphicsPipeline(const Device& device, const GraphicsPipelineSpecification& specs);
        ~VulkanGraphicsPipeline();

        // Getters
        inline const GraphicsPipelineSpecification& GetSpecification() const { return m_Specification; }

        // Internal getters
        inline VkPipeline GetVkPipeline() const { return m_Pipeline; }
        inline VkPipelineLayout GetVkPipelineLayout() const { return m_PipelineLayout; }

    private:
        GraphicsPipelineSpecification m_Specification;

        VkPipeline m_Pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Helper methods
    ////////////////////////////////////////////////////////////////////////////////////

}