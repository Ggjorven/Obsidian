#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/BufferSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanBuffer.hpp"
#include "NanoGraphics/Platform/Dummy/DummyBuffer.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{

    class Device;

    ////////////////////////////////////////////////////////////////////////////////////
    // InputLayout
    ////////////////////////////////////////////////////////////////////////////////////
    class InputLayout : public Traits::NoCopy
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanInputLayout>,
            Types::EnumToType<Information::Structs::RenderingAPI::D3D12, Internal::DummyInputLayout>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyInputLayout>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyInputLayout>
        >;
    public:
        // Destructor
        ~InputLayout() = default;

    private:
        // Constructor
        InputLayout(const Device& device, std::span<const VertexAttributeSpecification> attributes)
            : m_InputLayout(device, attributes) {}

    private:
        Type m_InputLayout;

        friend class Device;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Buffer
    ////////////////////////////////////////////////////////////////////////////////////
    class Buffer : public Traits::NoCopy
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanBuffer>,
            Types::EnumToType<Information::Structs::RenderingAPI::D3D12, Internal::DummyBuffer>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyBuffer>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyBuffer>
        >;
    public:
        // Destructor
        ~Buffer() = default;

        // Getters
        inline const BufferSpecification& GetSpecification() const { return m_Buffer.GetSpecification(); }

    private:
        // Constructor
        Buffer(const Device& device, const BufferSpecification& specs)
            : m_Buffer(device, specs) {}

    private:
        Type m_Buffer;

        friend class Device;
    };

}