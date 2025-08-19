#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/FramebufferSpec.hpp"

#include "Obsidian/Platform/Dx12/Dx12.hpp"

#include <Nano/Nano.hpp>

namespace Obsidian
{
    class Renderpass;
}

namespace Obsidian::Internal
{

    class Dx12Renderpass;
    class Dx12Framebuffer;

#if defined(OB_API_DX12)
    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12Framebuffer
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12Framebuffer
    {
    public:
        // Constructors & Destructor
        Dx12Framebuffer(const Renderpass& renderpass, const FramebufferSpecification& specs);
        ~Dx12Framebuffer();

        // Move functions
        Dx12Framebuffer(Dx12Framebuffer&& other) noexcept;
        Dx12Framebuffer& operator = (Dx12Framebuffer&& other) noexcept;

        // Copy functions
        Dx12Framebuffer(const Dx12Framebuffer& other);
        Dx12Framebuffer& operator = (const Dx12Framebuffer& other);

        // Methods
        void Resize();

        // Getters
        inline const FramebufferSpecification& GetSpecification() const { return m_Specification; }

        // Internal getters
        inline const Dx12Renderpass& GetDx12Renderpass() const { return *m_Renderpass; }

    private:
        const Dx12Renderpass* m_Renderpass; // Note: This is a pointer to allow copying of the object
        FramebufferSpecification m_Specification;
    };
#endif

}