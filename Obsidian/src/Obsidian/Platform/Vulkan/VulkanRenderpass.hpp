#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/API.hpp"
#include "Obsidian/Renderer/RenderpassSpec.hpp"
#include "Obsidian/Renderer/Framebuffer.hpp"

#include "Obsidian/Platform/Vulkan/Vulkan.hpp"

#include <Nano/Nano.hpp>

namespace Obsidian
{
    class Device;
    class Framebuffer;
}

namespace Obsidian::Internal
{

    class VulkanDevice;
    class VulkanRenderpass;

#if defined(OB_API_VULKAN)
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

        inline Framebuffer& GetFramebuffer(uint8_t frame) { return m_Framebuffers[frame]; }

        // Internal getters
        inline VkRenderPass GetVkRenderPass() const { return m_Renderpass; }

        inline const VulkanDevice& GetVulkanDevice() const { return m_Device; }
        inline Nano::Memory::StaticVector<Nano::Memory::DeferredConstruct<Framebuffer>, Information::MaxImageCount>& GetFramebuffers() { return m_Framebuffers; }

    private:
        const VulkanDevice& m_Device;
        RenderpassSpecification m_Specification;

        VkRenderPass m_Renderpass = VK_NULL_HANDLE;

        Nano::Memory::StaticVector<Nano::Memory::DeferredConstruct<Framebuffer>, Information::MaxImageCount> m_Framebuffers;
    };
#endif

}