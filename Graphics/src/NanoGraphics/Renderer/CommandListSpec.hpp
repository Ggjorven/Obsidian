#pragma once

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"

#include <Nano/Nano.hpp>

#include <cstdint>
#include <cmath>
#include <string>

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    //
    ////////////////////////////////////////////////////////////////////////////////////
    enum class CommandQueue : uint8_t
    {
        Graphics = 0,
        Compute,
        Present,

        Count
    };

    enum class PipelineStage : uint32_t
    {
        None = 0,

        TopOfPipe = 1 << 0,
        DrawIndirect = 1 << 1,
        VertexInput = 1 << 2,
        VertexShader = 1 << 3,
        TessellationControlShader = 1 << 4,
        TessellationEvaluationShader = 1 << 5,
        GeometryShader = 1 << 6,
        FragmentShader = 1 << 7,
        EarlyFragmentTests = 1 << 8,
        LateFragmentTests = 1 << 9,
        ColourAttachmentOutput = 1 << 10,
        ComputeShader = 1 << 11,
        Transfer = 1 << 12,
        BottomOfPipe = 1 << 13,
        Host = 1 << 14,
        AllGraphics = 1 << 15,
        AllCommands = 1 << 16,

        CommandPreprocess = 1 << 17,
        ConditionalRendering = 1 << 18,
        TaskShader = 1 << 19,
        MeshShader = 1 << 20,
        RayTracingShader = 1 << 21,
        FragmentShadingRateAttachment = 1 << 22,
        FragmentDensityProcess = 1 << 23,
        AccelerationStructureBuild = 1 << 24,
        TransformFeedback = 1 << 25,
    };

    NANO_DEFINE_BITWISE(PipelineStage)

    ////////////////////////////////////////////////////////////////////////////////////
    // CommandListSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct CommandListSpecification
    {
    public:
        std::string_view DebugName = {};

    public:
        // Setters
        inline constexpr CommandListSpecification& SetDebugName(std::string_view name) { DebugName = name; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // CommandListPoolSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct CommandListPoolSpecification
    {
    public:
        std::string_view DebugName = {};

    public:
        // Setters
        inline constexpr CommandListPoolSpecification& SetDebugName(std::string_view name) { DebugName = name; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // CommandListSubmitArgs
    ////////////////////////////////////////////////////////////////////////////////////
    struct CommandListSubmitArgs
    {
    public:
        CommandQueue Queue = CommandQueue::Graphics;
        PipelineStage WaitStage = PipelineStage::ColourAttachmentOutput;

    public:
        // Setters
        inline constexpr CommandListSubmitArgs& SetQueue(CommandQueue queue) { Queue = queue; return *this; }
    };

}