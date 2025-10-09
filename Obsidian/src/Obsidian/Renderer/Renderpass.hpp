#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/API.hpp"
#include "Obsidian/Renderer/RenderpassSpec.hpp"

#include "Obsidian/Platform/Vulkan/VulkanRenderpass.hpp"
#include "Obsidian/Platform/Dx12/Dx12Renderpass.hpp"
#include "Obsidian/Platform/Dummy/DummyRenderpass.hpp"

#include <Nano/Nano.hpp>

namespace Obsidian
{

    class Device;

    ////////////////////////////////////////////////////////////////////////////////////
    // Renderpass
    ////////////////////////////////////////////////////////////////////////////////////
    class Renderpass
    {
    public:
        using Type = Nano::Types::SelectorType<Information::RenderingAPI,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanRenderpass>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12Renderpass>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyRenderpass>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyRenderpass>
        >;
    public:
        // Destructor
        ~Renderpass() = default;

        // Methods
        inline Framebuffer& CreateFramebuffer(const FramebufferSpecification& specs) { return m_Impl->CreateFramebuffer(specs); } // Note: Framebuffers are stored in the Renderpass and will be destroyed when the renderpass is.

        inline void ResizeFramebuffers() { return m_Impl->ResizeFramebuffers(); }

        // Getters
        inline const RenderpassSpecification& GetSpecification() const { return m_Impl->GetSpecification(); }

        inline Framebuffer& GetFramebuffer(uint8_t frame) { return m_Impl->GetFramebuffer(frame); }

    public: //private:
        // Constructor
        inline Renderpass(const Device& device, const RenderpassSpecification& specs) { m_Impl.Construct(device, specs); }

    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class Device;
        friend class APICaster;
    };

}