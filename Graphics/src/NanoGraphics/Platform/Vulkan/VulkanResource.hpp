#pragma once

#include "NanoGraphics/Renderer/ResourceSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <cstdint>

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Mapping struct
    ////////////////////////////////////////////////////////////////////////////////////
    struct ResourceStateMapping
    {
    public:
        ResourceState State = ResourceState::Unknown;

        VkPipelineStageFlags2 StageFlags = VK_PIPELINE_STAGE_2_NONE;
        VkAccessFlags2 AccessMask = VK_ACCESS_2_NONE;
        VkImageLayout ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Mapping
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_ResourceStateMappings = std::to_array<ResourceStateMapping>({
        { ResourceState::Common,
            VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
            VK_ACCESS_2_NONE,
            VK_IMAGE_LAYOUT_UNDEFINED },
        { ResourceState::StorageBuffer,
            VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            VK_ACCESS_2_UNIFORM_READ_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED },
        { ResourceState::VertexBuffer,
            VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT,
            VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED },
        { ResourceState::IndexBuffer,
            VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT,
            VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT_KHR,
            VK_IMAGE_LAYOUT_UNDEFINED },
        { ResourceState::IndirectArgument,
            VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT,
            VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED },
        { ResourceState::ShaderResource,
            VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            VK_ACCESS_2_SHADER_READ_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
        { ResourceState::UnorderedAccess,
            VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
            VK_IMAGE_LAYOUT_GENERAL },
        { ResourceState::RenderTarget,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
        { ResourceState::DepthWrite,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL },
        { ResourceState::DepthRead,
            VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL },
        { ResourceState::StreamOut,
            VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT,
            VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,
            VK_IMAGE_LAYOUT_UNDEFINED },
        { ResourceState::CopyDest,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            VK_ACCESS_2_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL },
        { ResourceState::CopySource,
            VK_PIPELINE_STAGE_2_TRANSFER_BIT,
            VK_ACCESS_2_TRANSFER_READ_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL },
        { ResourceState::Present,
            VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            VK_ACCESS_2_MEMORY_READ_BIT,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR },
        //{ ResourceState::AccelStructRead,
        //    VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        //    VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR,
        //    VK_IMAGE_LAYOUT_UNDEFINED },
        //{ ResourceState::AccelStructWrite,
        //    VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        //    VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
        //    VK_IMAGE_LAYOUT_UNDEFINED },
        //{ ResourceState::AccelStructBuildInput,
        //    VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        //    VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR,
        //    VK_IMAGE_LAYOUT_UNDEFINED },
        //{ ResourceState::AccelStructBuildBlas,
        //    VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        //    VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR,
        //    VK_IMAGE_LAYOUT_UNDEFINED },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////////////////////
    // Convertion functions
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr VkObjectType ResourceTypeToVkObjectType(ResourceType type)
    {
        switch (type)
        {
        case ResourceType::Image:       return VK_OBJECT_TYPE_IMAGE;
        case ResourceType::ImageView:   return VK_OBJECT_TYPE_IMAGE_VIEW;
        case ResourceType::Sampler:     return VK_OBJECT_TYPE_SAMPLER;

        default:
            break;
        }

        return VK_OBJECT_TYPE_UNKNOWN;
    }

}