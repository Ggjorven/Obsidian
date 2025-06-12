#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/BufferSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanBuffer.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{

    class Device;

    ////////////////////////////////////////////////////////////////////////////////////
    // Framebuffer
    ////////////////////////////////////////////////////////////////////////////////////
    class Buffer : public Traits::NoCopy
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanBuffer>
        >;
    public:
        // Destructor
        ~Buffer() = default;

        // Getters
        inline const BufferSpecification& GetSpecification() const { return m_Device.GetSpecification(); }

    private:
        // Constructor
        Buffer(const Device& device, const BufferSpecification& specs)
            : m_Device(device, specs) {}

    private:
        Type m_Device;

        friend class Device;
    };

}