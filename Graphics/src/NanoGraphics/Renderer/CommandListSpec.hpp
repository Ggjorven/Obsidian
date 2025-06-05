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
        CommandQueue Queue = CommandQueue::Graphics;

    public:
        // Setters
        inline constexpr CommandListSpecification& SetQueue(CommandQueue queue) { Queue = queue; return *this; }
    };

}