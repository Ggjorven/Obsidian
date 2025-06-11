#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/RenderpassSpec.hpp"
#include "NanoGraphics/Renderer/FramebufferSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanFramebuffer.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{
    class Device;
    class Framebuffer;
}

namespace Nano::Graphics::Internal
{

    class VulkanDevice;

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanRenderpass
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanRenderpass : public Traits::NoCopy
    {
    public:
        // Constructors & Destructor
        VulkanRenderpass(const Device& device, const RenderpassSpecification& specs);
        ~VulkanRenderpass();

        // Methods
        Framebuffer& CreateFramebuffer(const FramebufferSpecification& specs);

        // Getters
        inline const RenderpassSpecification& GetSpecification() const { return m_Specification; }

        inline Framebuffer& GetFramebuffer(uint8_t frame) { return *reinterpret_cast<Framebuffer*>(&m_Framebuffers[frame]); }

        // Internal getters
        inline VkRenderPass GetVkRenderPass() const { return m_Renderpass; }

        inline const VulkanDevice& GetVulkanDevice() const { return m_Device; }
        inline Nano::Memory::StaticVector<VulkanFramebuffer, Information::BackBufferCount>& GetVulkanFramebuffers() { return m_Framebuffers; }

    private:
        const VulkanDevice& m_Device;
        RenderpassSpecification m_Specification;

        VkRenderPass m_Renderpass = VK_NULL_HANDLE;

        Nano::Memory::StaticVector<VulkanFramebuffer, Information::BackBufferCount> m_Framebuffers;
    };

}