#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/DeviceSpec.hpp"
#include "NanoGraphics/Renderer/CommandList.hpp"
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

        // Methods 
        inline void Wait() const { m_Device.Wait(); } // Note: Makes the CPU wait on the GPU to finish all operations

        // Creation methods // Note: Copy elision (RVO/NRVO) ensures object is constructed directly in the caller's stack frame.
        inline CommandListPool AllocateCommandListPool() const { return CommandListPool(*this); }

        inline Image CreateImage(const ImageSpecification& specs) const { return Image(*this, specs); }

    private:
        Type m_Device;
    };

}