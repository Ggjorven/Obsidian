#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/API.hpp"
#include "Obsidian/Renderer/FramebufferSpec.hpp"

#include "Obsidian/Platform/Vulkan/Vulkan.hpp"

#include <Nano/Nano.hpp>

namespace Obsidian
{
    class Renderpass;
}

namespace Obsidian::Internal
{

    class VulkanRenderpass;
    class VulkanFramebuffer;

#if defined(OB_API_VULKAN)
    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanFramebuffer
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanFramebuffer
    {
    public:
        // Constructors & Destructor
        VulkanFramebuffer(const Renderpass& renderpass, const FramebufferSpecification& specs);
        ~VulkanFramebuffer();

        // Move functions
        VulkanFramebuffer(VulkanFramebuffer&& other) noexcept;
        VulkanFramebuffer& operator = (VulkanFramebuffer&& other) noexcept;

        // Copy functions
        VulkanFramebuffer(const VulkanFramebuffer& other);
        VulkanFramebuffer& operator = (const VulkanFramebuffer& other);

        // Methods
        void Resize();

        // Getters
        inline const FramebufferSpecification& GetSpecification() const { return m_Specification; }

        // Internal getters
        inline VkFramebuffer GetVkFramebuffer() const { return m_Framebuffer; }

        inline const VulkanRenderpass& GetVulkanRenderpass() const { return *m_Renderpass; }

    private:
        // Private methods
        void Create();

    private:
        const VulkanRenderpass* m_Renderpass; // Note: This is a pointer to allow copying of the object
        FramebufferSpecification m_Specification;

        VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
    };
#endif

}