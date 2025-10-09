#include "obpch.h"
#include "Dx12Framebuffer.hpp"

#include "Obsidian/Core/Logging.hpp"
#include "Obsidian/Core/Information.hpp"
#include "Obsidian/Utils/Profiler.hpp"

#include "Obsidian/Platform/Dx12/Dx12Device.hpp"
#include "Obsidian/Platform/Dx12/Dx12Image.hpp"
#include "Obsidian/Platform/Dx12/Dx12Resources.hpp"
#include "Obsidian/Platform/Dx12/Dx12Renderpass.hpp"

namespace Obsidian::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12Framebuffer::Dx12Framebuffer(const Renderpass& renderpass, const FramebufferSpecification& specs)
        : m_Renderpass(api_cast<const Dx12Renderpass*>(&renderpass)), m_Specification(specs)
    {
        // Make sure the imageviews are available
        if (specs.ColourAttachment.IsValid())
        {
            Dx12Image& dxImage = *api_cast<Dx12Image*>(specs.ColourAttachment.ImagePtr);
            (void)dxImage.GetSubresourceView(specs.ColourAttachment.Subresources, ImageSubresourceViewUsage::RTV, ImageDimension::Unknown, Format::Unknown);
        }
        if (specs.DepthAttachment.IsValid())
        {
            Dx12Image& dxImage = *api_cast<Dx12Image*>(specs.DepthAttachment.ImagePtr);
            (void)dxImage.GetSubresourceView(specs.DepthAttachment.Subresources, ImageSubresourceViewUsage::DSV, ImageDimension::Unknown, Format::Unknown, specs.DepthAttachment.IsReadOnly);
        }
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