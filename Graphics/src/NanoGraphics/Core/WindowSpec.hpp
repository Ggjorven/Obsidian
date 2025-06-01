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
        // Window
        std::string_view Title = {};
        uint32_t Width = 0, Height = 0;

        EventCallbackFn EventCallback = nullptr;

        // Renderer
        bool VSync = false;
    };

}