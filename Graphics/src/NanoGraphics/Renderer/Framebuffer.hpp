#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/FramebufferSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanFramebuffer.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Framebuffer
    ////////////////////////////////////////////////////////////////////////////////////
    class Framebuffer : public Traits::NoCopy
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanFramebuffer>
        >;
    public:
        // Destructor
        ~Framebuffer() = default;

        // Methods
        inline void Resize() { m_Framebuffer.Resize(); }

        // Getters
        inline const FramebufferSpecification& GetSpecification() const { return m_Framebuffer.GetSpecification(); }

    private:
        // Constructor
        Framebuffer(const Renderpass& renderpass, const FramebufferSpecification& specs)
            : m_Framebuffer(renderpass, specs) {}

    private:
        Type m_Framebuffer;
    };

}