#include "ngpch.h"
#include "VulkanFramebuffer.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Image.hpp"
#include "NanoGraphics/Renderer/Renderpass.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanImage.hpp"

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanFramebuffer::VulkanFramebuffer(const Renderpass& renderpass, const FramebufferSpecification& specs)
        : m_Renderpass(api_cast<const VulkanRenderpass*>(&renderpass)), m_Specification(specs)
    {
        Create();
    }

    VulkanFramebuffer::~VulkanFramebuffer()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Move functions
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanFramebuffer::VulkanFramebuffer(VulkanFramebuffer&& other) noexcept
        : m_Renderpass(other.m_Renderpass), m_Specification(other.m_Specification), m_Framebuffer(other.m_Framebuffer)
    {
        other.m_Renderpass = nullptr;
        other.m_Specification = {};
        other.m_Framebuffer = VK_NULL_HANDLE;
    }

    VulkanFramebuffer& VulkanFramebuffer::operator = (VulkanFramebuffer&& other) noexcept
    {
        m_Renderpass = other.m_Renderpass;
        m_Specification = other.m_Specification;
        m_Framebuffer = other.m_Framebuffer;

        other.m_Renderpass = nullptr;
        other.m_Specification = {};
        other.m_Framebuffer = VK_NULL_HANDLE;

        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Copy functions
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanFramebuffer::VulkanFramebuffer(const VulkanFramebuffer& other)
        : m_Renderpass(other.m_Renderpass), m_Specification(other.m_Specification), m_Framebuffer(other.m_Framebuffer)
    {
    }

    VulkanFramebuffer& VulkanFramebuffer::operator = (const VulkanFramebuffer& other)
    {
        m_Renderpass = other.m_Renderpass;
        m_Specification = other.m_Specification;
        m_Framebuffer = other.m_Framebuffer;

        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanFramebuffer::Resize()
    {
        m_Renderpass->GetVulkanDevice().DestroyFramebuffer(*api_cast<Framebuffer*>(this));
        Create();
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanFramebuffer::Create()
    {
        Nano::Memory::StaticVector<VkImageView, 2> attachments; // Colour & Depth

        uint32_t width = 0, height = 0;
        uint32_t layers = 1;

        if (m_Specification.ColourAttachment.IsValid())
        {
            VulkanImage& vulkanImage = *api_cast<VulkanImage*>(m_Specification.ColourAttachment.ImagePtr);
            const ImageSpecification& imageSpec = m_Specification.ColourAttachment.ImagePtr->GetSpecification();
            ImageSubresourceSpecification resSubresources = ResolveImageSubresource(m_Specification.ColourAttachment.Subresources, imageSpec, false);

            width = std::max(imageSpec.Width >> resSubresources.BaseMipLevel, 1u);
            height = std::max(imageSpec.Height >> resSubresources.BaseMipLevel, 1u);
            layers = resSubresources.NumArraySlices;

            attachments.push_back(vulkanImage.GetSubresourceView(resSubresources, imageSpec.Dimension, imageSpec.ImageFormat, 0, ImageSubresourceViewType::AllAspects).GetVkImageView());
        }

        if (m_Specification.DepthAttachment.IsValid())
        {
            VulkanImage& vulkanImage = *api_cast<VulkanImage*>(m_Specification.DepthAttachment.ImagePtr);
            const ImageSpecification& imageSpec = m_Specification.DepthAttachment.ImagePtr->GetSpecification();
            ImageSubresourceSpecification resSubresources = ResolveImageSubresource(m_Specification.DepthAttachment.Subresources, imageSpec, false);

            // Validation checks
            if constexpr (Information::Validation)
            {
                if (width != 0 || height != 0) // If width/height are set, make sure the colour width/height is the same as the depth width/height
                {
                    NG_ASSERT((width == std::max(imageSpec.Width >> resSubresources.BaseMipLevel, 1u)), "[VkFramebuffer] Depth attachment's width doesn't match the colour attachment's width.");
                    NG_ASSERT((height == std::max(imageSpec.Height >> resSubresources.BaseMipLevel, 1u)), "[VkFramebuffer] Depth attachment's height doesn't match the colour attachment's height.");
                    NG_ASSERT((layers == resSubresources.NumArraySlices), "[VkFramebuffer] Depth attachment's arrayslices doesn't match the colour attachment's arrayslices.");
                }
            }

            width = std::max(imageSpec.Width >> resSubresources.BaseMipLevel, 1u);
            height = std::max(imageSpec.Height >> resSubresources.BaseMipLevel, 1u);
            layers = resSubresources.NumArraySlices;

            attachments.push_back(vulkanImage.GetSubresourceView(resSubresources, imageSpec.Dimension, imageSpec.ImageFormat, 0, ImageSubresourceViewType::AllAspects).GetVkImageView());
        }

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_Renderpass->GetVkRenderPass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = layers;

        VK_VERIFY(vkCreateFramebuffer(m_Renderpass->GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkDevice(), &framebufferInfo, VulkanAllocator::GetCallbacks(), &m_Framebuffer));

        if constexpr (Information::Validation)
        {
            if (!m_Specification.DebugName.empty())
                m_Renderpass->GetVulkanDevice().GetContext().SetDebugName(m_Framebuffer, VK_OBJECT_TYPE_FRAMEBUFFER, std::string(m_Specification.DebugName));
        }
    }

}