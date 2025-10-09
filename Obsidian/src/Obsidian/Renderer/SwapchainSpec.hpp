#pragma once

#include "Obsidian/Maths/Structs.hpp"

#include "Obsidian/Renderer/ResourceSpec.hpp"
#include "Obsidian/Renderer/ImageSpec.hpp"

#include <cstdint>
#include <cmath>
#include <limits>
#include <numeric>
#include <string>

namespace Obsidian
{

    class Window;

    ////////////////////////////////////////////////////////////////////////////////////
    // Flags
    ////////////////////////////////////////////////////////////////////////////////////
    enum class ColourSpace : uint8_t
    {
        SRGB = 0,
        HDR,
        LinearSRGB,
        DisplayNative,
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // SwapchainSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct SwapchainSpecification
    {
    public:
        Window* WindowTarget = nullptr;

        Format RequestedFormat = Format::BGRA8Unorm;
        ColourSpace RequestedColourSpace = ColourSpace::SRGB;

        bool VSync = false;

        std::string DebugName = {};

    public:
        // Setters
        inline constexpr SwapchainSpecification& SetWindow(Window& window) { WindowTarget = &window; return *this; }
        //inline constexpr SwapchainSpecification& SetWidth(uint32_t width) { Width = width; return *this; }
        //inline constexpr SwapchainSpecification& SetHeight(uint32_t height) { Height = height; return *this; }
        //inline constexpr SwapchainSpecification& SetWidthAndHeight(uint32_t width, uint32_t height) { Width = width; Height = height; return *this; }
        inline constexpr SwapchainSpecification& SetFormat(Format format) { RequestedFormat = format; return *this; }
        inline constexpr SwapchainSpecification& SetColourSpace(ColourSpace space) { RequestedColourSpace = space; return *this; }
        inline constexpr SwapchainSpecification& SetVSync(bool enabled) { VSync = enabled; return *this; }
        inline SwapchainSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }
    };

}