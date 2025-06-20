#pragma once

#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Core/Logging.hpp"

#include "NanoGraphics/Renderer/RenderpassSpec.hpp"
#include "NanoGraphics/Renderer/FramebufferSpec.hpp"

#include "NanoGraphics/Platform/Dummy/DummyFramebuffer.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{
    class Device;
    class Framebuffer;
}

namespace Nano::Graphics::Internal
{

    class DummyRenderpass;

#if 1 //defined(NG_API_DUMMY)
    ////////////////////////////////////////////////////////////////////////////////////
    // DummyRenderpass
    ////////////////////////////////////////////////////////////////////////////////////
    class DummyRenderpass
    {
    public:
        // Constructors & Destructor
        inline DummyRenderpass(const Device& device, const RenderpassSpecification& specs)
            : m_Specification(specs) { (void)device; }
        ~DummyRenderpass() = default;

        // Methods
        inline Framebuffer& CreateFramebuffer(const FramebufferSpecification& specs)
        {
            NG_ASSERT((m_Framebuffers.size() < m_Framebuffers.max_size()), "[DummyRenderpass] Can't create more framebuffers than backbuffers/swapchain images.");

            DummyFramebuffer framebuffer(*reinterpret_cast<Renderpass*>(this), specs);

            m_Framebuffers.push_back(std::move(framebuffer));
            return *reinterpret_cast<Framebuffer*>(&m_Framebuffers.back());
        }

        inline void ResizeFramebuffers()
        {
            for (auto& framebuffer : m_Framebuffers)
                framebuffer.Resize();
        }

        // Getters
        inline constexpr const RenderpassSpecification& GetSpecification() const { return m_Specification; }

        inline Framebuffer& GetFramebuffer(uint8_t frame) { return *reinterpret_cast<Framebuffer*>(&m_Framebuffers[frame]); }

    private:
        RenderpassSpecification m_Specification;

        Nano::Memory::StaticVector<DummyFramebuffer, Information::BackBufferCount> m_Framebuffers;
    };
#endif

}