#include "ngpch.h"
#include "VulkanRenderpass.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/Framebuffer.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanImage.hpp"

#include <Nano/Nano.hpp>

#include <optional>

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanRenderpass::VulkanRenderpass(const Device& device, const RenderpassSpecification& specs)
        : m_Device(*api_cast<const VulkanDevice*>(&device)), m_Specification(specs)
    {
        Nano::Memory::StaticVector<VkAttachmentDescription2, 2> attachments;
        std::optional<VkAttachmentReference2> colourReference;
        std::optional<VkAttachmentReference2> depthReference;

        if ((m_Specification.ColourSpecification.Width != 0) || (m_Specification.ColourSpecification.Height != 0))
        {
            VkAttachmentDescription2& attachment = attachments.emplace_back();
            attachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
            attachment.format = FormatToVkFormat(m_Specification.ColourSpecification.ImageFormat);
            attachment.samples = static_cast<VkSampleCountFlagBits>(SampleCountToVkSampleCountFlags(m_Specification.ColourSpecification.SampleCount));
            attachment.loadOp = LoadOperationToVkLoadOperation(m_Specification.ColourLoadOperation);
            attachment.storeOp = StoreOperationToVkStoreOperation(m_Specification.ColourStoreOperation);
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout = ResourceStateToImageLayout(m_Specification.ColourImageStartState);
            attachment.finalLayout = ResourceStateToImageLayout(m_Specification.ColourImageEndState);

            VkAttachmentReference2& reference = colourReference.emplace();
            reference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
            reference.attachment = static_cast<uint32_t>((attachments.size() - 1));
            reference.layout = ResourceStateToImageLayout(m_Specification.ColourImageRenderingState);
        }

        if ((m_Specification.DepthSpecification.Width != 0) || (m_Specification.DepthSpecification.Height != 0))
        {
            VkAttachmentDescription2& attachment = attachments.emplace_back();
            attachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
            attachment.format = FormatToVkFormat(m_Specification.DepthSpecification.ImageFormat);
            attachment.samples = static_cast<VkSampleCountFlagBits>(SampleCountToVkSampleCountFlags(m_Specification.DepthSpecification.SampleCount));
            attachment.loadOp = LoadOperationToVkLoadOperation(m_Specification.DepthLoadOperation);
            attachment.storeOp = StoreOperationToVkStoreOperation(m_Specification.DepthStoreOperation);
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachment.initialLayout = ResourceStateToImageLayout(m_Specification.DepthImageStartState);
            attachment.finalLayout = ResourceStateToImageLayout(m_Specification.DepthImageEndState);

            VkAttachmentReference2& reference = depthReference.emplace();
            reference.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
            reference.attachment = static_cast<uint32_t>((attachments.size() - 1));
            reference.layout = ResourceStateToImageLayout(m_Specification.DepthImageRenderingState);
        }

        VkSubpassDescription2 subpass = {};
        subpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
        subpass.pipelineBindPoint = PipelineBindpointToVkBindpoint(m_Specification.Bindpoint);
        subpass.colorAttachmentCount = (colourReference.has_value() ? 1 : 0);
        subpass.pColorAttachments = (colourReference.has_value() ? &colourReference.value() : 0);
        subpass.pDepthStencilAttachment = (depthReference.has_value() ? &depthReference.value() : nullptr);
        
        VkRenderPassCreateInfo2 renderpassInfo = {};
        renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
        renderpassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderpassInfo.pAttachments = attachments.data();
        renderpassInfo.subpassCount = 1;
        renderpassInfo.pSubpasses = &subpass;
        renderpassInfo.dependencyCount = 0;
        renderpassInfo.pDependencies = nullptr;
        
        VK_VERIFY(vkCreateRenderPass2(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &renderpassInfo, VulkanAllocator::GetCallbacks(), &m_Renderpass));
    
        if constexpr (Information::Validation)
        {
            if (!m_Specification.DebugName.empty())
                m_Device.GetContext().SetDebugName(m_Renderpass, VK_OBJECT_TYPE_RENDER_PASS, std::string(m_Specification.DebugName));
        }
    }

    VulkanRenderpass::~VulkanRenderpass()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    Framebuffer& VulkanRenderpass::CreateFramebuffer(const FramebufferSpecification& specs)
    {
        NG_ASSERT((m_Framebuffers.size() < m_Framebuffers.max_size()), "[VkRenderpass] Can't create more framebuffers than backbuffers/swapchain images.");

        Nano::Memory::DeferredConstruct<Framebuffer>& framebuffer = m_Framebuffers.emplace_back();
        framebuffer.Construct(*api_cast<const Renderpass*>(this), specs);
        return framebuffer;
    }

    void VulkanRenderpass::ResizeFramebuffers()
    {
        for (auto& framebuffer : m_Framebuffers)
            framebuffer->Resize();
    }

}