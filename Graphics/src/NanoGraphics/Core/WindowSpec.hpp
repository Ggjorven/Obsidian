#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

#include "NanoGraphics/Core/Events.hpp"
#include "NanoGraphics/Core/Logging.hpp"

namespace Nano::Graphics
{

    using EventCallbackFn = std::function<void(Event e)>;

    ////////////////////////////////////////////////////////////////////////////////////
    // WindowSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct WindowSpecification 
    {
    public:
        std::string_view Title = {};
        uint32_t Width = 0, Height = 0;

        EventCallbackFn EventCallback = nullptr;

    public:
        // Setters
        inline WindowSpecification& SetTitle(std::string_view title) { Title = title; return *this; }
        inline WindowSpecification& SetWidth(uint32_t width) { Width = width; return *this; }
        inline WindowSpecification& SetHeight(uint32_t height) { Height = height; return *this; }
        inline WindowSpecification& SetWidthAndHeight(uint32_t width, uint32_t height) { Width = width; Height = height; return *this; }
        inline WindowSpecification& SetEventCallback(EventCallbackFn callback) { EventCallback = callback; return *this; }
    };

}