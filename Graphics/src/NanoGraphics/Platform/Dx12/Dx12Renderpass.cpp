#include "ngpch.h"
#include "Dx12Renderpass.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/Framebuffer.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12Device.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12Renderpass::Dx12Renderpass(const Device& device, const RenderpassSpecification& specs)
        : m_Device(*api_cast<const Dx12Device*>(&device)), m_Specification(specs)
    {
       
    }

    Dx12Renderpass::~Dx12Renderpass()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    Framebuffer& Dx12Renderpass::CreateFramebuffer(const FramebufferSpecification& specs)
    {
        NG_ASSERT((m_Framebuffers.size() < m_Framebuffers.max_size()), "[Dx12Renderpass] Can't create more framebuffers than backbuffers/swapchain images.");

        Nano::Memory::DeferredConstruct<Framebuffer>& framebuffer = m_Framebuffers.emplace_back();
        framebuffer.Construct(*api_cast<const Renderpass*>(this), specs);
        return framebuffer;
    }

    void Dx12Renderpass::ResizeFramebuffers()
    {
    }

}