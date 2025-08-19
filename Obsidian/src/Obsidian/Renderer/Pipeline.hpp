#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/API.hpp"
#include "Obsidian/Renderer/PipelineSpec.hpp"

#include "Obsidian/Platform/Vulkan/VulkanPipeline.hpp"
#include "Obsidian/Platform/Dx12/Dx12Pipeline.hpp"
#include "Obsidian/Platform/Dummy/DummyPipeline.hpp"

#include <Nano/Nano.hpp>

namespace Obsidian
{

    class Device;

    ////////////////////////////////////////////////////////////////////////////////////
    // GraphicsPipeline
    ////////////////////////////////////////////////////////////////////////////////////
    class GraphicsPipeline
    {
    public:
        using Type = Nano::Types::SelectorType<Information::RenderingAPI,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanGraphicsPipeline>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12GraphicsPipeline>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyGraphicsPipeline>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyGraphicsPipeline>
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
        using Type = Nano::Types::SelectorType<Information::RenderingAPI,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanComputePipeline>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::DummyComputePipeline>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyComputePipeline>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyComputePipeline>
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