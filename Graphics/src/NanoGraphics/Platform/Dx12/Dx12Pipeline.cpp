#include "ngpch.h"
#include "Dx12Pipeline.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12Device.hpp"

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12GraphicsPipeline::Dx12GraphicsPipeline(const Device& device, const GraphicsPipelineSpecification& specs)
        : m_Specification(specs)
    {
        const Dx12Device& dxDevice = *api_cast<const Dx12Device*>(&device);
    }

    Dx12GraphicsPipeline::~Dx12GraphicsPipeline()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12ComputePipeline::Dx12ComputePipeline(const Device& device, const ComputePipelineSpecification& specs)
        : m_Specification(specs)
    {
        const Dx12Device& dxDevice = *api_cast<const Dx12Device*>(&device);

    }

    Dx12ComputePipeline::~Dx12ComputePipeline()
    {
    }

}