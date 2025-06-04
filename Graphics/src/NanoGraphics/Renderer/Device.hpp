#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/DeviceSpec.hpp"
#include "NanoGraphics/Renderer/Image.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Device
    ////////////////////////////////////////////////////////////////////////////////////
    class Device
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanDevice>
        >;
    public:
        // Constructor & Destructor
        inline Device(const DeviceSpecification& specs)
            : m_Device(specs) {}
        ~Device() = default;

        // Creation methods // Note: Copy elision (RVO/NRVO) ensures object is constructed directly in the caller's stack frame.
        Image CreateImage(const ImageSpecification& specs) const;

    private:
        Type m_Device;
    };

}