#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/API.hpp"
#include "Obsidian/Renderer/PipelineSpec.hpp"

#include "Obsidian/Platform/Vulkan/Vulkan.hpp"

#include <Nano/Nano.hpp>

namespace Obsidian
{
    class Device;
}

namespace Obsidian::Internal
{

    class VulkanDevice;
    class VulkanGraphicsPipeline;
    class VulkanComputePipeline;

#if defined(OB_API_VULKAN)
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

        inline VkShaderStageFlags GetPushConstantsStage() const { return m_PushConstantsStage; }

    private:
        GraphicsPipelineSpecification m_Specification;

        VkPipeline m_Pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

        VkShaderStageFlags m_PushConstantsStage = VK_SHADER_STAGE_ALL;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanComputePipeline
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanComputePipeline
    {
    public:
        // Constructors & Destructor
        VulkanComputePipeline(const Device& device, const ComputePipelineSpecification& specs);
        ~VulkanComputePipeline();

        // Getters
        inline const ComputePipelineSpecification& GetSpecification() const { return m_Specification; }

        // Internal getters
        inline VkPipeline GetVkPipeline() const { return m_Pipeline; }
        inline VkPipelineLayout GetVkPipelineLayout() const { return m_PipelineLayout; }

        inline VkShaderStageFlags GetPushConstantsStage() const { return m_PushConstantsStage; }

    private:
        ComputePipelineSpecification m_Specification;

        VkPipeline m_Pipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

        VkShaderStageFlags m_PushConstantsStage = VK_SHADER_STAGE_ALL;
    };
#endif

}