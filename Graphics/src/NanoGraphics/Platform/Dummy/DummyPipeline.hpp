#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/PipelineSpec.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{
    class Device;
}

namespace Nano::Graphics::Internal
{

    class DummyGraphicsPipeline;

#if 1 //defined(NG_API_DUMMY)
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
#endif

}