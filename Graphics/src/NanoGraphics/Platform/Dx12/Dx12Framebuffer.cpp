#include "ngpch.h"
#include "Dx12Framebuffer.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12Device.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Resources.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Renderpass.hpp"

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12Framebuffer::Dx12Framebuffer(const Renderpass& renderpass, const FramebufferSpecification& specs)
        : m_Renderpass(api_cast<const Dx12Renderpass*>(&renderpass)), m_Specification(specs)
    {
    }

    Dx12Framebuffer::~Dx12Framebuffer()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Move functions
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12Framebuffer::Dx12Framebuffer(Dx12Framebuffer&& other) noexcept
        : m_Renderpass(other.m_Renderpass), m_Specification(other.m_Specification)
    {
        other.m_Renderpass = nullptr;
        other.m_Specification = {};
    }

    Dx12Framebuffer& Dx12Framebuffer::operator = (Dx12Framebuffer&& other) noexcept
    {
        m_Renderpass = other.m_Renderpass;
        m_Specification = other.m_Specification;

        other.m_Renderpass = nullptr;
        other.m_Specification = {};

        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Copy functions
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12Framebuffer::Dx12Framebuffer(const Dx12Framebuffer& other)
        : m_Renderpass(other.m_Renderpass), m_Specification(other.m_Specification)
    {
    }

    Dx12Framebuffer& Dx12Framebuffer::operator = (const Dx12Framebuffer& other)
    {
        m_Renderpass = other.m_Renderpass;
        m_Specification = other.m_Specification;

        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12Framebuffer::Resize()
    {
    }

}