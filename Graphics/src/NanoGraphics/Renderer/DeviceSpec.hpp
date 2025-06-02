#pragma once

#include "NanoGraphics/Core/Events.hpp"
#include "NanoGraphics/Core/Logging.hpp"

#include <cstdint>
#include <functional>
#include <string_view>

namespace Nano::Graphics
{

    enum class DeviceMessage : uint8_t { Trace = 0, Info, Warn, Error };

    using DeviceMessageCallback = std::function<void(DeviceMessage error, const std::string& message)>;

    ////////////////////////////////////////////////////////////////////////////////////
    // DeviceSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct DeviceSpecification
    {
    public:
        void* NativeWindow;
        DeviceMessageCallback MessageCallback;
        std::span<const char*> Extensions; // Vulkan specific (SwapChain and MacOS related extensions included by default)

    public:
        // Setters
        inline DeviceSpecification& SetNativeWindow(void* nativeWindow) { NativeWindow = nativeWindow; return *this; }
        inline DeviceSpecification& SetMessageCallback(DeviceMessageCallback messageCallback) { MessageCallback = messageCallback; return *this; }
        inline DeviceSpecification& SetExtensions(std::span<const char*> extensions) { Extensions = extensions; return *this; }
    };

}