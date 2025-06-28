#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/PipelineSpec.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"

namespace Nano::Graphics
{
    class Device;
}

namespace Nano::Graphics::Internal
{

    class Dx12Device;
    class Dx12GraphicsPipeline;
    class Dx12ComputePipeline;

#if defined(NG_API_DX12)
    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12GraphicsPipeline
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12GraphicsPipeline
    {
    public:
        // Constructor & Destructor
        Dx12GraphicsPipeline(const Device& device, const GraphicsPipelineSpecification& specs);
        ~Dx12GraphicsPipeline();

        // Getters
        inline const GraphicsPipelineSpecification& GetSpecification() const { return m_Specification; }

    private:
        GraphicsPipelineSpecification m_Specification;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12ComputePipeline
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12ComputePipeline
    {
    public:
        // Constructors & Destructor
        Dx12ComputePipeline(const Device& device, const ComputePipelineSpecification& specs);
        ~Dx12ComputePipeline();

        // Getters
        inline const ComputePipelineSpecification& GetSpecification() const { return m_Specification; }

    private:
        ComputePipelineSpecification m_Specification;
    };
#endif

}