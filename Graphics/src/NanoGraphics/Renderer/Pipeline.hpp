#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/PipelineSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanPipeline.hpp"
#include "NanoGraphics/Platform/Dummy/DummyPipeline.hpp"

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
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanGraphicsPipeline>,
            Types::EnumToType<Information::Structs::RenderingAPI::D3D12, Internal::DummyGraphicsPipeline>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyGraphicsPipeline>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyGraphicsPipeline>
        >;
    public:
        // Destructor
        ~GraphicsPipeline() = default;

    public: //private:
        // Constructor
        inline GraphicsPipeline(const Device& device, const GraphicsPipelineSpecification& specs)
            : m_GraphicsPipeline(device, specs) {}

    private:
        Type m_GraphicsPipeline;

        friend class Device;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ComputePipeline
    ////////////////////////////////////////////////////////////////////////////////////
    class ComputePipeline
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanComputePipeline>,
            Types::EnumToType<Information::Structs::RenderingAPI::D3D12, Internal::DummyComputePipeline>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyComputePipeline>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyComputePipeline>
        >;
    public:
        // Destructor
        ~ComputePipeline() = default;

    public: //private:
        // Constructor
        inline ComputePipeline(const Device& device, const ComputePipelineSpecification& specs)
            : m_ComputePipeline(device, specs) {}

    private:
        Type m_ComputePipeline;

        friend class Device;
    };

}