#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/PipelineSpec.hpp"

#include <Nano/Nano.hpp>

namespace Obsidian
{
    class Device;
}

namespace Obsidian::Internal
{

    class DummyGraphicsPipeline;
    class DummyComputePipeline;

#if 1 //defined(OB_API_DUMMY)
    ////////////////////////////////////////////////////////////////////////////////////
    // DummyGraphicsPipeline
    ////////////////////////////////////////////////////////////////////////////////////
    class DummyGraphicsPipeline
    {
    public:
        // Constructors & Destructor
        inline constexpr DummyGraphicsPipeline(const Device& device, const GraphicsPipelineSpecification& specs)
            : m_Specification(specs) { (void)device; }
        constexpr ~DummyGraphicsPipeline() = default;

        // Getters
        inline const GraphicsPipelineSpecification& GetSpecification() const { return m_Specification; }

    private:
        GraphicsPipelineSpecification m_Specification;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // DummyComputePipeline
    ////////////////////////////////////////////////////////////////////////////////////
    class DummyComputePipeline
    {
    public:
        // Constructors & Destructor
        DummyComputePipeline(const Device& device, const ComputePipelineSpecification& specs) 
            : m_Specification(specs) { (void)device; }
        constexpr ~DummyComputePipeline() = default;

        // Getters
        inline constexpr const ComputePipelineSpecification& GetSpecification() const { return m_Specification; }

    private:
        ComputePipelineSpecification m_Specification;
    };
#endif

}