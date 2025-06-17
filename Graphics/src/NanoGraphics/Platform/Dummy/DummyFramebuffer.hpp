#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/FramebufferSpec.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{
    class Renderpass;
}

namespace Nano::Graphics::Internal
{

    class DummyFramebuffer;

#if 1 //defined(NG_API_DUMMY)
    ////////////////////////////////////////////////////////////////////////////////////
    // DummyFramebuffer
    ////////////////////////////////////////////////////////////////////////////////////
    class DummyFramebuffer
    {
    public:
        // Constructors & Destructor
        constexpr DummyFramebuffer() = default;
        inline constexpr DummyFramebuffer(const Renderpass& renderpass, const FramebufferSpecification& specs)
            : m_Specification(specs) { (void)renderpass; }
        constexpr ~DummyFramebuffer() = default;

        // Move functions
        inline constexpr DummyFramebuffer(DummyFramebuffer&& other) noexcept = default;
        inline constexpr DummyFramebuffer& operator = (DummyFramebuffer&& other) noexcept = default;

        // Copy functions
        inline constexpr DummyFramebuffer(const DummyFramebuffer& other) = default;
        inline constexpr DummyFramebuffer& operator = (const DummyFramebuffer& other) = default;

        // Methods
        inline constexpr void Resize() {}

        // Getters
        inline constexpr const FramebufferSpecification& GetSpecification() const { return m_Specification; }

    private:
        FramebufferSpecification m_Specification;
    };
#endif

}