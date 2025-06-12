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
        inline const VulkanDevice& GetVulkanDevice() const { return m_Device; }

    private:
        const VulkanDevice& m_Device;
        GraphicsPipelineSpecification m_Specification;
    };

}