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
        std::string DebugName = {};

    public:
        // Setters
        inline CommandListSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // CommandListPoolSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct CommandListPoolSpecification
    {
    public:
        std::string DebugName = {};

    public:
        // Setters
        inline CommandListPoolSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }
    };

}