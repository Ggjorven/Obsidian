#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
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
    class InputLayout
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

    public: //private:
        // Constructor
        inline InputLayout(const Device& device, std::span<const VertexAttributeSpecification> attributes) { m_InputLayout.Construct(device, attributes); }

    private:
        // Helper getter
        inline Type& APICasterGet() { return m_InputLayout.Get(); }

    private:
        Internal::APIObject<Type> m_InputLayout = {};

        friend class Device;
        friend class APICaster;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Buffer
    ////////////////////////////////////////////////////////////////////////////////////
    class Buffer
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
        inline const BufferSpecification& GetSpecification() const { return m_Buffer->GetSpecification(); }

    public: //private:
        // Constructor
        inline Buffer(const Device& device, const BufferSpecification& specs) { m_Buffer.Construct(device, specs); }

    private:
        // Helper getter
        inline Type& APICasterGet() { return m_Buffer.Get(); }

    private:
        Internal::APIObject<Type> m_Buffer = {};

        friend class Device;
        friend class APICaster;
    };

}