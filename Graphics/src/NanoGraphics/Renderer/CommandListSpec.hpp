#pragma once

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"

#include <cstdint>
#include <cmath>
#include <string>

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Flags
    ////////////////////////////////////////////////////////////////////////////////////
    enum class CommandQueue : uint8_t
    {
        Graphics = 0,
        Compute,
        Present
    };

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

    public:
        // Setters
        inline constexpr CommandListSubmitArgs& SetQueue(CommandQueue queue) { Queue = queue; return *this; }
    };

}