#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/FramebufferSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanFramebuffer.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Framebuffer.hpp"
#include "NanoGraphics/Platform/Dummy/DummyFramebuffer.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Framebuffer
    ////////////////////////////////////////////////////////////////////////////////////
    class Framebuffer
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanFramebuffer>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12Framebuffer>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyFramebuffer>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyFramebuffer>
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