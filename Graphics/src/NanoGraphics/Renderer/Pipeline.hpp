#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/PipelineSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanPipeline.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Pipeline.hpp"
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
            Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12GraphicsPipeline>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyGraphicsPipeline>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyGraphicsPipeline>
        >;
    public:
        // Destructor
        ~GraphicsPipeline() = default;

        // Getters
        inline const GraphicsPipelineSpecification& GetSpecification() const { return m_Impl->GetSpecification(); }

    public: //private:
        // Constructor
        inline GraphicsPipeline(const Device& device, const GraphicsPipelineSpecification& specs) { m_Impl.Construct(device, specs); }

    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class Device;
        friend class APICaster;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ComputePipeline
    ////////////////////////////////////////////////////////////////////////////////////
    class ComputePipeline
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanComputePipeline>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::DummyComputePipeline>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyComputePipeline>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyComputePipeline>
        >;
    public:
        // Destructor
        ~ComputePipeline() = default;

        // Getters
        inline const ComputePipelineSpecification& GetSpecification() const { return m_Impl->GetSpecification(); }

    public: //private:
        // Constructor
        inline ComputePipeline(const Device& device, const ComputePipelineSpecification& specs) { m_Impl.Construct(device, specs); }

    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class Device;
        friend class APICaster;
    };

}