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

    private:
        GraphicsPipelineSpecification m_Specification;
    };

}