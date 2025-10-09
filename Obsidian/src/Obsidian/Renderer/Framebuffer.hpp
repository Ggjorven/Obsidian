#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/API.hpp"
#include "Obsidian/Renderer/FramebufferSpec.hpp"

#include "Obsidian/Platform/Vulkan/VulkanFramebuffer.hpp"
#include "Obsidian/Platform/Dx12/Dx12Framebuffer.hpp"
#include "Obsidian/Platform/Dummy/DummyFramebuffer.hpp"

#include <Nano/Nano.hpp>

namespace Obsidian
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Framebuffer
    ////////////////////////////////////////////////////////////////////////////////////
    class Framebuffer
    {
    public:
        using Type = Nano::Types::SelectorType<Information::RenderingAPI,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanFramebuffer>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12Framebuffer>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyFramebuffer>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyFramebuffer>
        >;
    public:
        // Destructor
        ~Framebuffer() = default;

        // Methods
        inline void Resize() { m_Impl->Resize(); }

        // Getters
        inline const FramebufferSpecification& GetSpecification() const { return m_Impl->GetSpecification(); }

    public: //private:
        // Constructor
        inline Framebuffer(const Renderpass& renderpass, const FramebufferSpecification& specs) { m_Impl.Construct(renderpass, specs); }

    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class APICaster;
    };

}