#include "ngpch.h"
#include "VulkanPipeline.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/Pipeline.hpp"

namespace Nano::Graphics::Internal
{

    static_assert(std::is_same_v<GraphicsPipeline::Type, VulkanGraphicsPipeline>, "Current GraphicsPipeline::Type is not VulkanGraphicsPipeline and Vulkan source code is being compiled.");

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanGraphicsPipeline::VulkanGraphicsPipeline(const Device& device, const GraphicsPipelineSpecification& specs)
        : m_Specification(specs)
    {
        NG_ASSERT(false, "TODO");
        // TODO: ...
    }

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
    {
    }

}