#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/RenderpassSpec.hpp"
#include "NanoGraphics/Renderer/Framebuffer.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{
    class Device;
    class Framebuffer;
}

namespace Nano::Graphics::Internal
{

    class Dx12Device;
    class Dx12Renderpass;

#if defined(NG_API_DX12)
    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12Renderpass
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12Renderpass
    {
    public:
        // Constructors & Destructor
        Dx12Renderpass(const Device& device, const RenderpassSpecification& specs);
        ~Dx12Renderpass();

        // Methods
        Framebuffer& CreateFramebuffer(const FramebufferSpecification& specs);

        void ResizeFramebuffers();

        // Getters
        inline const RenderpassSpecification& GetSpecification() const { return m_Specification; }

        inline Framebuffer& GetFramebuffer(uint8_t frame) { return m_Framebuffers[frame]; }

        // Internal getters
        inline const Dx12Device& GetDx12Device() const { return m_Device; }
        inline Nano::Memory::StaticVector<Nano::Memory::DeferredConstruct<Framebuffer>, Information::MaxImageCount>& GetFramebuffers() { return m_Framebuffers; }

    private:
        const Dx12Device& m_Device;
        RenderpassSpecification m_Specification;

        Nano::Memory::StaticVector<Nano::Memory::DeferredConstruct<Framebuffer>, Information::MaxImageCount> m_Framebuffers;
    };
#endif

}