#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/RenderpassSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanRenderpass.hpp"
#include "NanoGraphics/Platform/Dummy/DummyRenderpass.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{

    class Device;

    ////////////////////////////////////////////////////////////////////////////////////
    // Renderpass
    ////////////////////////////////////////////////////////////////////////////////////
    class Renderpass
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanRenderpass>,
            Types::EnumToType<Information::Structs::RenderingAPI::D3D12, Internal::DummyRenderpass>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyRenderpass>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyRenderpass>
        >;
    public:
        // Destructor
        ~Renderpass() = default;

        // Methods
        inline Framebuffer& CreateFramebuffer(const FramebufferSpecification& specs) { return m_Renderpass->CreateFramebuffer(specs); } // Note: Framebuffers are stored in the Renderpass and will be destroyed when the renderpass is.

        inline void ResizeFramebuffers() { return m_Renderpass->ResizeFramebuffers(); }

        // Getters
        inline const RenderpassSpecification& GetSpecification() const { return m_Renderpass->GetSpecification(); }

        inline Framebuffer& GetFramebuffer(uint8_t frame) { return m_Renderpass->GetFramebuffer(frame); }

    public: //private:
        // Constructor
        inline Renderpass(const Device& device, const RenderpassSpecification& specs) { m_Renderpass.Construct(device, specs); }

    private:
        // Helper getter
        inline Type& APICasterGet() { return m_Renderpass.Get(); }

    private:
        Internal::APIObject<Type> m_Renderpass = {};

        friend class Device;
        friend class APICaster;
    };

}