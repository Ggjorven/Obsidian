#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/API.hpp"
#include "Obsidian/Renderer/BufferSpec.hpp"

#include "Obsidian/Platform/Vulkan/VulkanBuffer.hpp"
#include "Obsidian/Platform/Dx12/Dx12Buffer.hpp"
#include "Obsidian/Platform/Dummy/DummyBuffer.hpp"

#include <Nano/Nano.hpp>

namespace Obsidian
{

    class Device;

    ////////////////////////////////////////////////////////////////////////////////////
    // InputLayout
    ////////////////////////////////////////////////////////////////////////////////////
    class InputLayout
    {
    public:
        using Type = Nano::Types::SelectorType<Information::RenderingAPI,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanInputLayout>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12InputLayout>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyInputLayout>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyInputLayout>
        >;
    public:
        // Destructor
        ~InputLayout() = default;

    public: //private:
        // Constructor
        inline InputLayout(const Device& device, std::span<const VertexAttributeSpecification> attributes) { m_Impl.Construct(device, attributes); }

    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class Device;
        friend class APICaster;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Buffer
    ////////////////////////////////////////////////////////////////////////////////////
    class Buffer
    {
    public:
        using Type = Nano::Types::SelectorType<Information::RenderingAPI,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanBuffer>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12Buffer>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyBuffer>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyBuffer>
        >;
    public:
        // Destructor
        ~Buffer() = default;

        // Getters
        inline const BufferSpecification& GetSpecification() const { return m_Impl->GetSpecification(); }

        inline size_t GetAlignment() const { return m_Impl->GetAlignment(); }

    public: //private:
        // Constructor
        inline Buffer(const Device& device, const BufferSpecification& specs) { m_Impl.Construct(device, specs); }

    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class Device;
        friend class APICaster;
    };

}