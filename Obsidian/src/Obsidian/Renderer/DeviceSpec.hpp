#pragma once

#include <cstdint>
#include <span>
#include <functional>

namespace Obsidian
{

    enum class DeviceMessageType : uint8_t { Trace = 0, Info, Warn, Error };

    using DeviceMessageCallback = std::function<void(DeviceMessageType error, const std::string& message)>;

    using DeviceDestroyFn = std::function<void()>;
    using DeviceDestroyCallback = std::function<void(DeviceDestroyFn destroyObjectFn)>;

    ////////////////////////////////////////////////////////////////////////////////////
    // DeviceSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct DeviceSpecification
    {
    public:
        void* NativeWindow = nullptr; // Note: Just creating a device with one window is fine, the device can be used across all created windows.
        
        DeviceMessageCallback MessageCallback = nullptr;
        DeviceDestroyCallback DestroyCallback = nullptr; // Note: Functions should be stored in a queue and executed at the end/begin of a frame to allow for finishing of resource usage.

        std::span<const char*> Extensions = {}; // Vulkan specific (SwapChain and MacOS related extensions included by default)

    public:
        // Setters
        inline constexpr DeviceSpecification& SetNativeWindow(void* nativeWindow) { NativeWindow = nativeWindow; return *this; }
        inline DeviceSpecification& SetMessageCallback(DeviceMessageCallback messageCallback) { MessageCallback = messageCallback; return *this; }
        inline DeviceSpecification& SetDestroyCallback(DeviceDestroyCallback destroyCallback) { DestroyCallback = destroyCallback; return *this; }
        inline constexpr DeviceSpecification& SetExtensions(std::span<const char*> extensions) { Extensions = extensions; return *this; }
    };

}