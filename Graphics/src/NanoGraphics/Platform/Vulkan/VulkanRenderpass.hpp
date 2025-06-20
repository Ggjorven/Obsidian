#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/RenderpassSpec.hpp"
#include "NanoGraphics/Renderer/Framebuffer.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{
    class Device;
    class Framebuffer;
}

namespace Nano::Graphics::Internal
{

    class VulkanDevice;
    class VulkanRenderpass;

#if defined(NG_API_VULKAN)
    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanRenderpass
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanRenderpass
    {
    public:
        // Constructors & Destructor
        VulkanRenderpass(const Device& device, const RenderpassSpecification& specs);
        ~VulkanRenderpass();

        // Methods
        Framebuffer& CreateFramebuffer(const FramebufferSpecification& specs);

        void ResizeFramebuffers();

        // Getters
        inline const RenderpassSpecification& GetSpecification() const { return m_Specification; }

        inline Framebuffer& GetFramebuffer(uint8_t frame) { return *api_cast<Framebuffer*>(&m_Framebuffers[frame]); }

        // Internal getters
        inline VkRenderPass GetVkRenderPass() const { return m_Renderpass; }

        inline const VulkanDevice& GetVulkanDevice() const { return m_Device; }
        inline Nano::Memory::StaticVector<VulkanFramebuffer, Information::BackBufferUpperLimit>& GetVulkanFramebuffers() { return m_Framebuffers; }

    private:
        const VulkanDevice& m_Device;
        RenderpassSpecification m_Specification;

        VkRenderPass m_Renderpass = VK_NULL_HANDLE;

        Nano::Memory::StaticVector<VulkanFramebuffer, Information::BackBufferUpperLimit> m_Framebuffers;
    };
#endif

}