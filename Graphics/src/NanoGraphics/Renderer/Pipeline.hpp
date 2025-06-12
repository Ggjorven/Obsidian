#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/PipelineSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanPipeline.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{

    class Device;

    ////////////////////////////////////////////////////////////////////////////////////
    // GraphicsPipeline
    ////////////////////////////////////////////////////////////////////////////////////
    class GraphicsPipeline
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanGraphicsPipeline>
        >;
    public:
        // Destructor
        ~GraphicsPipeline() = default;

    private:
        // Constructor
        GraphicsPipeline(const Device& device, const GraphicsPipelineSpecification& specs)
            : m_GraphicsPipeline(device, specs) {}

    private:
        Type m_GraphicsPipeline;

        friend class Device;
    };

}