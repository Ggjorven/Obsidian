#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <functional>

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
        void* NativeWindow = nullptr;
        DeviceMessageCallback MessageCallback = nullptr;
        std::span<const char*> Extensions = {}; // Vulkan specific (SwapChain and MacOS related extensions included by default)

    public:
        // Setters
        inline constexpr DeviceSpecification& SetNativeWindow(void* nativeWindow) { NativeWindow = nativeWindow; return *this; }
        inline DeviceSpecification& SetMessageCallback(DeviceMessageCallback messageCallback) { MessageCallback = messageCallback; return *this; }
        inline constexpr DeviceSpecification& SetExtensions(std::span<const char*> extensions) { Extensions = extensions; return *this; }
    };

}