#pragma once

#include "Obsidian/Core/Events.hpp"
#include "Obsidian/Core/Logging.hpp"

#include <Nano/Nano.hpp>

#include <cstdint>
#include <functional>
#include <string_view>

namespace Obsidian
{

    using EventCallbackFn = std::function<void(Event e)>;

    ////////////////////////////////////////////////////////////////////////////////////
    // Flags
    ////////////////////////////////////////////////////////////////////////////////////
    enum class WindowFlags : uint16_t
    {
        None = 0,

        Resizable = 1 << 0,
        Decorated = 1 << 1,
        Titlebar = Decorated,
        Visible = 1 << 2,
        Show = Visible, 
        Maximized = 1 << 3,
        Floating = 1 << 4,
        AlwaysOnTop = Floating,
        Transparent = 1 << 5,
        Focused = 1 << 6,
        FocusOnShow = 1 << 7,
        CenterCursor = 1 << 8,

        Default = Resizable | Decorated | Visible | Focused | FocusOnShow
    };

    NANO_DEFINE_BITWISE(WindowFlags)

    ////////////////////////////////////////////////////////////////////////////////////
    // WindowSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct WindowSpecification 
    {
    public:
        std::string Title = {};
        uint32_t Width = 0, Height = 0;
        WindowFlags Flags = WindowFlags::None;

        EventCallbackFn EventCallback = nullptr;

    public:
        // Setters
        inline WindowSpecification& SetTitle(const std::string& title) { Title = title; return *this; }
        inline constexpr WindowSpecification& SetWidth(uint32_t width) { Width = width; return *this; }
        inline constexpr WindowSpecification& SetHeight(uint32_t height) { Height = height; return *this; }
        inline constexpr WindowSpecification& SetWidthAndHeight(uint32_t width, uint32_t height) { Width = width; Height = height; return *this; }
        inline constexpr WindowSpecification& SetFlags(WindowFlags flags) { Flags = flags; return *this; }
        inline constexpr WindowSpecification& AddFlags(WindowFlags flags) { Flags |= flags; return *this; }
        inline WindowSpecification& SetEventCallback(EventCallbackFn callback) { EventCallback = callback; return *this; }
    };

}