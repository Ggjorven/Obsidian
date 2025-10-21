#include "obpch.h"
#include "VulkanCommandList.hpp"

#include "Obsidian/Core/Logging.hpp"
#include "Obsidian/Utils/Profiler.hpp"

#include "Obsidian/Renderer/Device.hpp"
#include "Obsidian/Renderer/CommandList.hpp"
#include "Obsidian/Renderer/Swapchain.hpp"
#include "Obsidian/Renderer/Pipeline.hpp"

#include "Obsidian/Platform/Vulkan/VulkanDevice.hpp"
#include "Obsidian/Platform/Vulkan/VulkanPipeline.hpp"

namespace Obsidian::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanCommandListPool::VulkanCommandListPool(Swapchain& swapchain, const CommandListPoolSpecification& specs)
        : m_Swapchain(*api_cast<VulkanSwapchain*>(&swapchain)), m_Specification(specs)
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Note: Allows us to reset the command buffer and reuse it.
        poolInfo.queueFamilyIndex = m_Swapchain.GetVulkanDevice().GetContext().GetVulkanPhysicalDevice().GetQueueFamilyIndices().QueueFamily;
        
        VK_VERIFY(vkCreateCommandPool(m_Swapchain.GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkDevice(), &poolInfo, VulkanAllocator::GetCallbacks(), &m_CommandPool));

        if constexpr (Information::Validation)
        {
            if (!m_Specification.DebugName.empty())
                m_Swapchain.GetVulkanDevice().GetContext().SetDebugName(m_CommandPool, VK_OBJECT_TYPE_COMMAND_POOL, std::string(m_Specification.DebugName));
        }
    }

    VulkanCommandListPool::~VulkanCommandListPool()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanCommandListPool::FreeList(CommandList& list) const
    {
        const VulkanContext& context = m_Swapchain.GetVulkanDevice().GetContext();

        VkDevice device = context.GetVulkanLogicalDevice().GetVkDevice();
        VkCommandBuffer commandBuffer = (*api_cast<VulkanCommandList*>(&list)).GetVkCommandBuffer();
        m_Swapchain.GetVulkanDevice().GetContext().Destroy([device, commandPool = m_CommandPool, commandBuffer]() mutable
        { 
            vkFreeCommandBuffers(device, commandPool, 1ul, &commandBuffer);
        });
    }

    void VulkanCommandListPool::FreeLists(std::span<CommandList*> lists) const
    {
        std::vector<VkCommandBuffer> commandBuffers;
        commandBuffers.reserve(lists.size());

        for (auto list : lists)
        {
            VkCommandBuffer commandBuffer = (*api_cast<VulkanCommandList*>(list)).GetVkCommandBuffer();
            commandBuffers.push_back(commandBuffer);
        }

        VkDevice device = m_Swapchain.GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkDevice();
        m_Swapchain.GetVulkanDevice().GetContext().Destroy([device, commandPool = m_CommandPool, commandBuffers = std::move(commandBuffers)]() mutable
        {
            vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        });
    }

    void VulkanCommandListPool::Reset() const
    {
        vkResetCommandPool(m_Swapchain.GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkDevice(), m_CommandPool, 0);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanCommandList::VulkanCommandList(CommandListPool& pool, const CommandListSpecification& specs)
        : m_Pool(*api_cast<VulkanCommandListPool*>(&pool)), m_Specification(specs)
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_Pool.GetVkCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VK_VERIFY(vkAllocateCommandBuffers(m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkDevice(), &allocInfo, &m_CommandBuffer));

        if constexpr (Information::Validation)
        {
            if (!m_Specification.DebugName.empty())
                m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetContext().SetDebugName(m_CommandBuffer, VK_OBJECT_TYPE_COMMAND_BUFFER, std::format("CommandList \"{0}\" from: {1}", m_Specification.DebugName, m_Pool.GetSpecification().DebugName));
        }
    }

    VulkanCommandList::~VulkanCommandList()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanCommandList::Open()
    {
        OB_PROFILE("VulkanCommandList::Open()");
        m_WaitStage = VK_PIPELINE_STAGE_2_NONE;

        {
            OB_PROFILE("VulkanCommandList::Open::Begin");
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            VK_VERIFY(vkBeginCommandBuffer(m_CommandBuffer, &beginInfo));
        }
    }

    void VulkanCommandList::Close()
    {
        OB_PROFILE("VulkanCommandList::Close()");
        VK_VERIFY(vkEndCommandBuffer(m_CommandBuffer));

        m_CurrentGraphicsPipeline = nullptr;
        m_CurrentComputePipeline = nullptr;
    }

    void VulkanCommandList::Submit(const CommandListSubmitArgs& args) 
    {
        OB_PROFILE("VulkanCommandBuffer::Submit()");

        VulkanSwapchain& swapchain = m_Pool.GetVulkanSwapchain();

        std::span<const CommandList*> waitOn;
        std::visit([&](auto&& arg)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::vector<const CommandList*>>)
                waitOn = const_cast<std::vector<const CommandList*>&>(arg); // Note: This is worst thing I have done in my life. I can never recover.
            else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::span<const CommandList*>>)
                waitOn = arg;
        }, args.WaitOnLists);

        std::vector<VkSemaphoreSubmitInfo> waitInfos;
        waitInfos.reserve(waitOn.size() + (args.WaitForSwapchainImage ? 1ull : 0ull));

        // Wait semaphores
        if (args.WaitForSwapchainImage)
        {
            VkSemaphoreSubmitInfo& info = waitInfos.emplace_back();
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            info.semaphore = swapchain.GetVkImageAvailableSemaphore(swapchain.GetCurrentFrame());
            info.stageMask = (m_WaitStage == VK_PIPELINE_STAGE_2_NONE ? VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT : m_WaitStage);
            info.value = 0ull;
        }
        for (const CommandList* list : waitOn)
        {
            VkSemaphoreSubmitInfo& info = waitInfos.emplace_back();
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            info.semaphore = swapchain.GetVkTimelineSemaphore();
            info.stageMask = m_WaitStage;
            info.value = swapchain.GetPreviousCommandListWaitValue(*api_cast<const VulkanCommandList*>(list));
        }

        // Signal semaphores
        std::vector<VkSemaphoreSubmitInfo> signalInfos;
        signalInfos.reserve(1ull + (args.OnFinishMakeSwapchainPresentable ? 1ull : 0ull));

        VkSemaphoreSubmitInfo& timelineInfo = signalInfos.emplace_back();
        timelineInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        timelineInfo.semaphore = swapchain.GetVkTimelineSemaphore();
        timelineInfo.value = swapchain.RetrieveCommandListWaitValue(*this);

        if (args.OnFinishMakeSwapchainPresentable)
        {
            VkSemaphoreSubmitInfo& info = signalInfos.emplace_back();
            info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            info.semaphore = swapchain.GetVkSwapchainPresentableSemaphore(swapchain.GetAcquiredImage());
            info.stageMask = m_WaitStage;
            info.value = 0ull;
        }

        // Command info
        VkCommandBufferSubmitInfo commandInfo = {};
        commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandInfo.commandBuffer = m_CommandBuffer;

        // Submit info
        VkSubmitInfo2 submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

        submitInfo.waitSemaphoreInfoCount = static_cast<uint32_t>(waitInfos.size());
        submitInfo.pWaitSemaphoreInfos = waitInfos.data();

        submitInfo.commandBufferInfoCount = 1ul;
        submitInfo.pCommandBufferInfos = &commandInfo;

        submitInfo.signalSemaphoreInfoCount = static_cast<uint32_t>(signalInfos.size());
        submitInfo.pSignalSemaphoreInfos = signalInfos.data();
        
#if defined(OB_PLATFORM_APPLE)
        VK_VERIFY(VkExtension::g_vkQueueSubmit2KHR(m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkQueue(args.Queue), 1, &submitInfo, nullptr));
#else
        VK_VERIFY(vkQueueSubmit2(m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkQueue(m_Pool.GetSpecification().Queue), 1, &submitInfo, nullptr));
#endif
    }

    void VulkanCommandList::WaitTillComplete() const
    {
        OB_PROFILE("VulkanCommandList::WaitTillComplete()");
        VkSemaphore semaphore = m_Pool.GetVulkanSwapchain().GetVkTimelineSemaphore();
        uint64_t value = m_Pool.GetVulkanSwapchain().GetPreviousCommandListWaitValue(*this);

        VkSemaphoreWaitInfo waitInfo = {};
        waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        waitInfo.semaphoreCount = 1;
        waitInfo.pSemaphores = &semaphore;
        waitInfo.pValues = &value;

        vkWaitSemaphores(m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkDevice(), &waitInfo, std::numeric_limits<uint64_t>::max());
    }

    void VulkanCommandList::CommitBarriers()
    {
        OB_PROFILE("VulkanCommandList::CommitBarriers()");

        auto& imageBarriers = m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetTracker().GetImageBarriers(*api_cast<const CommandList*>(this));
        auto& bufferBarriers = m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetTracker().GetBufferBarriers(*api_cast<const CommandList*>(this));

        if (imageBarriers.empty() && bufferBarriers.empty())
            return;

        std::vector<VkImageMemoryBarrier2> vkImageBarriers;
        vkImageBarriers.reserve(imageBarriers.size());
        std::vector<VkBufferMemoryBarrier2> vkBufferBarriers;
        vkBufferBarriers.reserve(bufferBarriers.size());

        for (const ImageBarrier& imageBarrier : imageBarriers)
        {
            const ResourceStateMapping& before = ResourceStateToMapping(imageBarrier.StateBefore);
            const ResourceStateMapping& after = ResourceStateToMapping(imageBarrier.StateAfter);

            OB_ASSERT((after.ImageLayout != VK_IMAGE_LAYOUT_UNDEFINED), "[VkCommandList] Can't transition to undefined layout.");

            Image& image = *imageBarrier.ImagePtr;
            VulkanImage& vulkanImage = *api_cast<VulkanImage*>(imageBarrier.ImagePtr);

            VkImageMemoryBarrier2& barrier2 = vkImageBarriers.emplace_back();
            barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            barrier2.srcStageMask = before.StageFlags;
            barrier2.dstStageMask = after.StageFlags;
            barrier2.srcAccessMask = before.AccessMask;
            barrier2.dstAccessMask = after.AccessMask;
            barrier2.oldLayout = before.ImageLayout;
            barrier2.newLayout = after.ImageLayout;
            barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier2.image = vulkanImage.GetVkImage();

            barrier2.subresourceRange.aspectMask = VkFormatToImageAspect(FormatToVkFormat(image.GetSpecification().ImageFormat));
            barrier2.subresourceRange.baseMipLevel = (imageBarrier.EntireTexture ? 0 : imageBarrier.ImageMipLevel);
            barrier2.subresourceRange.levelCount = (imageBarrier.EntireTexture ? image.GetSpecification().MipLevels : 1);
            barrier2.subresourceRange.baseArrayLayer = (imageBarrier.EntireTexture ? 0 : imageBarrier.ImageArraySlice);
            barrier2.subresourceRange.layerCount = (imageBarrier.EntireTexture ? image.GetSpecification().ArraySize : 1);
        }

        for (const BufferBarrier& bufferBarrier : bufferBarriers)
        {
            const ResourceStateMapping& before = ResourceStateToMapping(bufferBarrier.StateBefore);
            const ResourceStateMapping& after = ResourceStateToMapping(bufferBarrier.StateAfter);

            Buffer& buffer = *bufferBarrier.BufferPtr;
            VulkanBuffer& vulkanBuffer = *api_cast<VulkanBuffer*>(&buffer);

            VkBufferMemoryBarrier2& barrier2 = vkBufferBarriers.emplace_back();
            barrier2.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
            barrier2.srcStageMask = before.StageFlags;
            barrier2.dstStageMask = after.StageFlags;
            barrier2.srcAccessMask = before.AccessMask;
            barrier2.dstAccessMask = after.AccessMask;
            barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier2.buffer = vulkanBuffer.GetVkBuffer();
            barrier2.offset = 0;
            barrier2.size = buffer.GetSpecification().Size;
        }

        if (!vkImageBarriers.empty() || !vkBufferBarriers.empty())
        {
            VkDependencyInfo dependencyInfo = {};
            dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            dependencyInfo.bufferMemoryBarrierCount = static_cast<uint32_t>(vkBufferBarriers.size());
            dependencyInfo.pBufferMemoryBarriers = vkBufferBarriers.data();
            dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(vkImageBarriers.size());
            dependencyInfo.pImageMemoryBarriers = vkImageBarriers.data();

#if defined(OB_PLATFORM_APPLE)
            VkExtension::g_vkCmdPipelineBarrier2KHR(m_CommandBuffer, &dependencyInfo);
#else
            vkCmdPipelineBarrier2(m_CommandBuffer, &dependencyInfo);
#endif
        }

        bufferBarriers.clear();
        imageBarriers.clear();
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Object methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanCommandList::StartRenderpass(const RenderpassStartArgs& args)
    {
        OB_PROFILE("VulkanCommandList::StartRenderpass()");

        OB_ASSERT(args.Pass, "[VkCommandList] No Renderpass passed in.");

        // Renderpass
        {
            OB_PROFILE("VulkanCommandList::StartRenderpass::Renderpass");

            VulkanRenderpass& renderpass = *api_cast<VulkanRenderpass*>(args.Pass);

            Framebuffer* framebuffer = args.Frame;
            if (!framebuffer)
            {
                OB_ASSERT((renderpass.GetFramebuffers().size() == m_Pool.GetVulkanSwapchain().GetImageCount()), "[VkCommandList] No framebuffer was passed into GraphicsState, but renderpass' framebuffer count doesn't align with swapchain image count.");
                framebuffer = &renderpass.GetFramebuffer(static_cast<uint8_t>(m_Pool.GetVulkanSwapchain().GetAcquiredImage()));
            }
            VulkanFramebuffer& vkFramebuffer = *api_cast<VulkanFramebuffer*>(framebuffer);

            // Make sure the attachments are in the begin state
            {
                if (framebuffer->GetSpecification().ColourAttachment.IsValid() && (renderpass.GetSpecification().ColourImageStartState != ResourceState::Unknown))
                {
                    const FramebufferAttachment& attachment = framebuffer->GetSpecification().ColourAttachment;
                    RequireState(*attachment.ImagePtr, attachment.Subresources, renderpass.GetSpecification().ColourImageStartState);
                }
                if (framebuffer->GetSpecification().DepthAttachment.IsValid() && (renderpass.GetSpecification().DepthImageStartState != ResourceState::Unknown))
                {
                    const FramebufferAttachment& attachment = framebuffer->GetSpecification().DepthAttachment;
                    RequireState(*attachment.ImagePtr, attachment.Subresources, renderpass.GetSpecification().DepthImageStartState);
                }
                CommitBarriers();
            }

            VkRenderPassBeginInfo renderpassInfo = {};
            renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderpassInfo.renderPass = renderpass.GetVkRenderPass();
            renderpassInfo.framebuffer = vkFramebuffer.GetVkFramebuffer();
            renderpassInfo.renderArea.offset = { 0, 0 };
            renderpassInfo.renderArea.extent = { static_cast<uint32_t>(args.ViewportState.GetWidth()), static_cast<uint32_t>(args.ViewportState.GetHeight()) };

            // Clear values
            Nano::Memory::StaticVector<VkClearValue, 2> clearValues;
            if (vkFramebuffer.GetSpecification().ColourAttachment.IsValid())
                clearValues.push_back(VkClearValue({ args.ColourClear.r, args.ColourClear.g, args.ColourClear.b, args.ColourClear.a }));
            if (vkFramebuffer.GetSpecification().DepthAttachment.IsValid())
                clearValues.push_back(VkClearValue({ args.DepthClear, 0 }));

            renderpassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderpassInfo.pClearValues = clearValues.data();

            VkSubpassBeginInfo subpassInfo = {};
            subpassInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO;
            subpassInfo.contents = VK_SUBPASS_CONTENTS_INLINE;

            {
                OB_PROFILE("VulkanCommandList::StartRenderpass::Begin");
                vkCmdBeginRenderPass2(m_CommandBuffer, &renderpassInfo, &subpassInfo);
            }
        }

        SetViewport(args.ViewportState);
        SetScissor(args.Scissor);
    }

    void VulkanCommandList::EndRenderpass(const RenderpassEndArgs& args)
    {
        OB_PROFILE("VulkanCommandList::EndRenderpass()");

        VkSubpassEndInfo endInfo = {};
        endInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO;

        vkCmdEndRenderPass2(m_CommandBuffer, &endInfo);

        // Refresh StateTrackers internal states to reflect the end states
        {
            VulkanRenderpass& renderpass = *api_cast<VulkanRenderpass*>(args.Pass);

            Framebuffer* framebuffer = args.Frame;
            if (!framebuffer)
            {
                OB_ASSERT((renderpass.GetFramebuffers().size() == m_Pool.GetVulkanSwapchain().GetImageCount()), "[VkCommandList] No framebuffer was passed into GraphicsState, but renderpass' framebuffer count doesn't align with swapchain image count.");
                framebuffer = &renderpass.GetFramebuffer(static_cast<uint8_t>(m_Pool.GetVulkanSwapchain().GetAcquiredImage()));
            }

            // Set the internal tracking state to reflect the actual end state
            {
                if (framebuffer->GetSpecification().ColourAttachment.IsValid())
                {
                    const FramebufferAttachment& attachment = framebuffer->GetSpecification().ColourAttachment;
                    m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetTracker().SetImageState(*attachment.ImagePtr, attachment.Subresources, renderpass.GetSpecification().ColourImageEndState);
                }
                if (framebuffer->GetSpecification().DepthAttachment.IsValid())
                {
                    const FramebufferAttachment& attachment = framebuffer->GetSpecification().DepthAttachment;
                    m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetTracker().SetImageState(*attachment.ImagePtr, attachment.Subresources, renderpass.GetSpecification().DepthImageEndState);
                }
            }
        }
    }

    void VulkanCommandList::SetViewport(const Viewport& viewport) const
    {
        OB_PROFILE("VulkanCommandList::SetViewport()");

        VkViewport vkViewport = {};
        vkViewport.x = viewport.MinX;
        vkViewport.y = viewport.MinY;
        vkViewport.width = viewport.GetWidth();
        vkViewport.height = viewport.GetHeight();
        vkViewport.minDepth = viewport.MinZ;
        vkViewport.maxDepth = viewport.MaxZ;
        vkCmdSetViewport(m_CommandBuffer, 0, 1, &vkViewport);
    }

    void VulkanCommandList::SetScissor(const ScissorRect& scissor) const
    {
        OB_PROFILE("VulkanCommandList::SetScissor()");
        VkRect2D vkScissor = {};
        vkScissor.offset = { scissor.MinX, scissor.MinY };
        vkScissor.extent = { static_cast<uint32_t>(scissor.GetWidth()), static_cast<uint32_t>(scissor.GetHeight()) };
        vkCmdSetScissor(m_CommandBuffer, 0, 1, &vkScissor);
    }

    void VulkanCommandList::BindPipeline(const GraphicsPipeline& pipeline)
    {
        OB_PROFILE("VulkanCommandList::BindPipeline()");

        m_CurrentGraphicsPipeline = &pipeline;
        m_CurrentComputePipeline = nullptr;
        
        const VulkanGraphicsPipeline& vulkanPipeline = *api_cast<const VulkanGraphicsPipeline*>(&pipeline);
        vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline.GetVkPipeline());
    }

    void VulkanCommandList::BindPipeline(const ComputePipeline& pipeline)
    {
        OB_PROFILE("VulkanCommandList::BindPipeline()");

        m_CurrentGraphicsPipeline = nullptr;
        m_CurrentComputePipeline = &pipeline;

        const VulkanComputePipeline& vulkanPipeline = *api_cast<const VulkanComputePipeline*>(&pipeline);
        vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, vulkanPipeline.GetVkPipeline());
    }

    void VulkanCommandList::BindBindingSet(const BindingSet& set, std::span<const uint32_t> dynamicOffsets)
    {
        OB_PROFILE("VulkanCommandList::BindBindingSet()");

        OB_ASSERT(m_CurrentGraphicsPipeline || m_CurrentComputePipeline, "[VkCommandList] A pipeline must be bound to bind a binding set.");

        VkPipelineLayout layout;
        VkPipelineBindPoint bindPoint;
        if (m_CurrentGraphicsPipeline)
        {
            layout = api_cast<const VulkanGraphicsPipeline*>(m_CurrentGraphicsPipeline)->GetVkPipelineLayout();
            bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        }
        else
        {
            layout = api_cast<const VulkanGraphicsPipeline*>(m_CurrentComputePipeline)->GetVkPipelineLayout();
            bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
        }

        const VulkanBindingSet& vkSet = *api_cast<const VulkanBindingSet*>(&set);
        VulkanBindingLayout& vkLayout = *api_cast<VulkanBindingLayout*>(vkSet.GetVulkanBindingSetPool().GetSpecification().Layout);
        VkDescriptorSet descriptorSet = vkSet.GetVkDescriptorSet();

        vkCmdBindDescriptorSets(m_CommandBuffer, bindPoint, layout, vkLayout.GetRegisterSpace(), 1, &descriptorSet, static_cast<uint32_t>(dynamicOffsets.size()), dynamicOffsets.data());
    }

    void VulkanCommandList::BindBindingSets(const std::span<const BindingSet*> sets, std::span<const std::span<const uint32_t>> dynamicOffsets)
    {
        OB_PROFILE("VulkanCommandList::BindBindingSets()");

        OB_ASSERT(m_CurrentGraphicsPipeline || m_CurrentComputePipeline, "[VkCommandList] A pipeline must be bound to bind a binding set.");
        OB_ASSERT((dynamicOffsets.empty()) || (sets.size() == dynamicOffsets.size()), "[VkCommandList] The amount of dynamic offsets spans must be the same as the amount of sets or be empty.");

        VkPipelineLayout layout;
        VkPipelineBindPoint bindPoint;
        if (m_CurrentGraphicsPipeline)
        {
            layout = api_cast<const VulkanGraphicsPipeline*>(m_CurrentGraphicsPipeline)->GetVkPipelineLayout();
            bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        }
        else
        {
            layout = api_cast<const VulkanGraphicsPipeline*>(m_CurrentComputePipeline)->GetVkPipelineLayout();
            bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
        }

        // Note: This corresponds to SetID               Sets               DynamicOffsets
        std::vector<std::tuple<uint32_t, std::vector<VkDescriptorSet>, std::span<const uint32_t>>> descriptorSetsSet;
        std::get<std::vector<VkDescriptorSet>>(descriptorSetsSet.emplace_back()).reserve(sets.size());

        // Runtime validation
        {
            for (size_t i = 0; i < sets.size(); i++)
            {
                if (sets[i] == nullptr)
                    continue;

                // If SetID doesn't match create a new starting point with current SetID
                if (std::get<uint32_t>(descriptorSetsSet.back()) != i)
                {
                    auto& tuple = descriptorSetsSet.emplace_back(
                        static_cast<uint32_t>(i),
                        std::vector<VkDescriptorSet>(),
                        ((!dynamicOffsets.empty()) ? dynamicOffsets[i] : std::span<const uint32_t>())
                    );

                    std::get<std::vector<VkDescriptorSet>>(tuple).reserve(sets.size());
                }

                // Add descriptor
                const VulkanBindingSet& vulkanSet = *api_cast<const VulkanBindingSet*>(sets[i]);
                std::get<std::vector<VkDescriptorSet>>(descriptorSetsSet.back()).push_back(vulkanSet.GetVkDescriptorSet());
            }
        }

        // Binding
        for (const auto& [setID, descriptorSets, dOffsets] : descriptorSetsSet)
            vkCmdBindDescriptorSets(m_CommandBuffer, bindPoint, layout, setID, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), static_cast<uint32_t>(dOffsets.size()), dOffsets.data());
    }

    void VulkanCommandList::BindVertexBuffer(const Buffer& buffer) const
    {
        OB_PROFILE("VulkanCommandList::BindVertexBuffer()");
        OB_ASSERT((buffer.GetSpecification().IsVertexBuffer), "[VkCommandList] To bind a buffer as a vertex buffer it must have been created with IsVertexBuffer equal to true.");
        const VulkanBuffer& vulkanBuffer = *api_cast<const VulkanBuffer*>(&buffer);

        VkBuffer vkBuffer = vulkanBuffer.GetVkBuffer();
        VkDeviceSize vkOffsets = 0;
        
        vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, &vkBuffer, &vkOffsets);
    }

    void VulkanCommandList::BindIndexBuffer(const Buffer& buffer) const
    {
        OB_PROFILE("VulkanCommandList::BindIndexBuffer()");
        OB_ASSERT((buffer.GetSpecification().IsIndexBuffer), "[VkCommandList] To bind a buffer as an index buffer it must have been created with IsIndexBuffer equal to true.");
        const VulkanBuffer& vulkanBuffer = *api_cast<const VulkanBuffer*>(&buffer);

        VkBuffer vkBuffer = vulkanBuffer.GetVkBuffer();

        vkCmdBindIndexBuffer(m_CommandBuffer, vkBuffer, 0, ((vulkanBuffer.GetSpecification().BufferFormat == Format::R16UInt) ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32));
    }

    void VulkanCommandList::CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, Image& src, const ImageSliceSpecification& srcSlice)
    {
        OB_PROFILE("VulkanCommandList::CopyImage()");

        OB_ASSERT(m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetTracker().Contains(dst), "[VkCommandList] Using an untracked image is not allowed, call StartTracking() on dst image.");
        OB_ASSERT(m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetTracker().Contains(src), "[VkCommandList] Using an untracked image is not allowed, call StartTracking() on src image.");

        SetWaitStage(VK_PIPELINE_STAGE_2_TRANSFER_BIT);

        VulkanImage& vulkanDst = *api_cast<VulkanImage*>(&dst);
        VulkanImage& vulkanSrc = *api_cast<VulkanImage*>(&src);

        ImageSliceSpecification resDstSlice = ResolveImageSlice(dstSlice, dst.GetSpecification());
        ImageSliceSpecification resSrcSlice = ResolveImageSlice(srcSlice, src.GetSpecification());

        ImageSubresourceSpecification srcSubresource = ImageSubresourceSpecification(
            resSrcSlice.ImageMipLevel, 1,
            resSrcSlice.ImageArraySlice, 1
        );

        VkFormat srcFormat = FormatToVkFormat(src.GetSpecification().ImageFormat);
        VkImageAspectFlags srcAspectFlags = GuessSubresourceImageAspectFlags(srcFormat, ImageSubresourceViewType::AllAspects);

        ImageSubresourceSpecification dstSubresource = ImageSubresourceSpecification(
            resDstSlice.ImageMipLevel, 1,
            resDstSlice.ImageArraySlice, 1
        );

        // Enforce permanent state
        //ResolvePermanentState(src, srcSubresource);
        //ResolvePermanentState(dst, dstSubresource);

        VkFormat dstFormat = FormatToVkFormat(dst.GetSpecification().ImageFormat);
        VkImageAspectFlags dstAspectFlags = GuessSubresourceImageAspectFlags(dstFormat, ImageSubresourceViewType::AllAspects);

        VkExtent3D extent = VkExtent3D(
            std::min(resSrcSlice.Width, resDstSlice.Width),
            std::min(resSrcSlice.Height, resDstSlice.Height),
            std::min(resSrcSlice.Depth, resDstSlice.Depth)
        );

        RequireState(src, ImageSubresourceSpecification(resSrcSlice.ImageMipLevel, 1, resSrcSlice.ImageArraySlice, 1), ResourceState::CopySrc);
        RequireState(dst, ImageSubresourceSpecification(resDstSlice.ImageMipLevel, 1, resDstSlice.ImageArraySlice, 1), ResourceState::CopyDst);
        CommitBarriers();

        VkCopyImageInfo2 copyInfo = {};
        copyInfo.sType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2;
        copyInfo.srcImage = vulkanSrc.GetVkImage();
        copyInfo.srcImageLayout = ResourceStateToImageLayout(ResourceState::CopySrc);
        copyInfo.dstImage = vulkanDst.GetVkImage();
        copyInfo.dstImageLayout = ResourceStateToImageLayout(ResourceState::CopyDst);
        copyInfo.regionCount = 1;

        VkImageCopy2 region = {};
        region.sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2;
        region.srcSubresource.aspectMask = srcAspectFlags;
        region.srcSubresource.mipLevel = srcSubresource.BaseMipLevel;
        region.srcSubresource.baseArrayLayer = srcSubresource.BaseArraySlice;
        region.srcSubresource.layerCount = srcSubresource.NumArraySlices;
        region.srcOffset = { resSrcSlice.X, resSrcSlice.Y, resSrcSlice.Z };

        region.dstSubresource.aspectMask = dstAspectFlags;
        region.dstSubresource.mipLevel = dstSubresource.BaseMipLevel;
        region.dstSubresource.baseArrayLayer = dstSubresource.BaseArraySlice;
        region.dstSubresource.layerCount = dstSubresource.NumArraySlices;
        region.dstOffset = { resDstSlice.X, resDstSlice.Y, resDstSlice.Z };

        region.extent = extent;

        copyInfo.pRegions = &region;

#if defined(OB_PLATFORM_APPLE)
        VkExtension::g_vkCmdCopyImage2KHR(m_CommandBuffer, &copyInfo);
#else
        vkCmdCopyImage2(m_CommandBuffer, &copyInfo);
#endif

        // Update back to permanent state
        m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetTracker().ResolvePermanentState(*api_cast<const CommandList*>(this), src, srcSubresource);
        m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetTracker().ResolvePermanentState(*api_cast<const CommandList*>(this), dst, dstSubresource);
        CommitBarriers();
    }

    void VulkanCommandList::CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, StagingImage& src, const ImageSliceSpecification& srcSlice)
    {
        OB_PROFILE("VulkanCommandList::CopyImage()");

        VulkanStagingImage& srcVulkanStagingImage = *api_cast<VulkanStagingImage*>(&src);
        VulkanBuffer& srcVulkanBuffer = api_cast<VulkanStagingImage*>(&src)->GetVulkanBuffer();
        VulkanImage& dstVulkanImage = *api_cast<VulkanImage*>(&dst);

        ImageSliceSpecification resSrcSlice = ResolveImageSlice(srcSlice, src.GetSpecification());
        ImageSliceSpecification resDstSlice = ResolveImageSlice(dstSlice, dst.GetSpecification());

        auto srcRegion = srcVulkanStagingImage.GetSliceRegion(resSrcSlice.ImageMipLevel, resSrcSlice.ImageArraySlice, resSrcSlice.Z);

        ImageSubresourceSpecification dstSubresource = ImageSubresourceSpecification(
            resDstSlice.ImageMipLevel, 1,
            resDstSlice.ImageArraySlice, 1
        );

        // Enforce permanent state
        //ResolvePermanentState(*api_cast<Buffer*>(&srcVulkanBuffer));
        //ResolvePermanentState(dst, dstSubresource);

        VkBufferImageCopy2 copyInfo = {};
        copyInfo.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
        copyInfo.bufferOffset = srcRegion.Offset;
        copyInfo.bufferRowLength = resSrcSlice.Width;
        copyInfo.bufferImageHeight = resSrcSlice.Height;

        copyInfo.imageSubresource.aspectMask = VkFormatToImageAspect(FormatToVkFormat(dst.GetSpecification().ImageFormat));
        copyInfo.imageSubresource.mipLevel = resDstSlice.ImageMipLevel;
        copyInfo.imageSubresource.baseArrayLayer = resDstSlice.ImageArraySlice;
        copyInfo.imageSubresource.layerCount = 1;

        copyInfo.imageOffset = { resDstSlice.X, resDstSlice.Y, resDstSlice.Z };
        copyInfo.imageExtent = { resDstSlice.Width, resDstSlice.Height, resDstSlice.Depth };

        RequireState(*api_cast<Buffer*>(&srcVulkanBuffer), ResourceState::CopySrc);
        RequireState(dst, dstSubresource, ResourceState::CopyDst);
        CommitBarriers();

        VkCopyBufferToImageInfo2 copyBufferToImageInfo = {};
        copyBufferToImageInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2;
        copyBufferToImageInfo.srcBuffer = srcVulkanBuffer.GetVkBuffer();
        copyBufferToImageInfo.dstImage = dstVulkanImage.GetVkImage();
        copyBufferToImageInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copyBufferToImageInfo.regionCount = 1;
        copyBufferToImageInfo.pRegions = &copyInfo;

#if defined(OB_PLATFORM_APPLE)
        VkExtension::g_vkCmdCopyBufferToImage2KHR(m_CommandBuffer, &copyBufferToImageInfo);
#else
        vkCmdCopyBufferToImage2(m_CommandBuffer, &copyBufferToImageInfo);
#endif

        // Update back to permanent state
        m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetTracker().ResolvePermanentState(*api_cast<const CommandList*>(this), *api_cast<Buffer*>(&srcVulkanBuffer));
        m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetTracker().ResolvePermanentState(*api_cast<const CommandList*>(this), dst, dstSubresource);
        CommitBarriers();
    }

    void VulkanCommandList::CopyBuffer(Buffer& dst, Buffer& src, size_t size, size_t srcOffset, size_t dstOffset)
    {
        OB_PROFILE("VulkanCommandList::CopyBuffer()");

        // Enforce permanent state
        //ResolvePermanentState(src);
        //ResolvePermanentState(dst);

        VulkanBuffer& dstVulkanBuffer = *api_cast<VulkanBuffer*>(&dst);
        VulkanBuffer& srcVulkanBuffer = *api_cast<VulkanBuffer*>(&src);

        RequireState(src, ResourceState::CopySrc);
        RequireState(dst, ResourceState::CopyDst);
        CommitBarriers();

        VkBufferCopy2 copyRegion = {};
        copyRegion.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
        copyRegion.srcOffset = srcOffset;
        copyRegion.dstOffset = dstOffset;
        copyRegion.size = size;

        VkCopyBufferInfo2 copyInfo = {};
        copyInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
        copyInfo.srcBuffer = srcVulkanBuffer.GetVkBuffer();
        copyInfo.dstBuffer = dstVulkanBuffer.GetVkBuffer();
        copyInfo.regionCount = 1;
        copyInfo.pRegions = &copyRegion;

#if defined(OB_PLATFORM_APPLE)
        VkExtension::g_vkCmdCopyBuffer2KHR(m_CommandBuffer, &copyInfo);
#else
        vkCmdCopyBuffer2(m_CommandBuffer, &copyInfo);
#endif

        // Update back to permanent state
        m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetTracker().ResolvePermanentState(*api_cast<const CommandList*>(this), src);
        m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetTracker().ResolvePermanentState(*api_cast<const CommandList*>(this), dst);
        CommitBarriers();
    }

    void VulkanCommandList::Dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ) const
    {
        OB_PROFILE("VulkanCommandList::Dispatch()");
        vkCmdDispatch(m_CommandBuffer, groupsX, groupsY, groupsZ);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // State methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanCommandList::RequireState(Image& image, const ImageSubresourceSpecification& subresources, ResourceState state)
    {
        m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetTracker().RequireImageState(*api_cast<const CommandList*>(this), image, subresources, state);
    }

    void VulkanCommandList::RequireState(Buffer& buffer, ResourceState state)
    {
        m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetTracker().RequireBufferState(*api_cast<const CommandList*>(this), buffer, state);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Draw methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanCommandList::DrawIndexed(const DrawArguments& args) const
    {
        OB_PROFILE("VulkanCommandList::DrawIndexed()");
        vkCmdDrawIndexed(m_CommandBuffer, args.VertexCount, args.InstanceCount, args.StartIndexLocation, args.StartVertexLocation, args.StartInstanceLocation);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Other methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanCommandList::PushConstants(const void* memory, size_t size, size_t srcOffset, size_t dstOffset)
    {
        OB_PROFILE("VulkanCommandList::PushConstants()");

        OB_ASSERT(m_CurrentGraphicsPipeline || m_CurrentComputePipeline, "[VkCommandList] Can't push constants if no pipeline is bound.");
        OB_ASSERT((size + dstOffset <= 128), "[VkCommandList] Size + dstOffset exceeds max push constants size (128 bytes)");
        OB_ASSERT((dstOffset % 4 == 0), "[VkCommandList] DstOffset must be aligned to 4 bytes.");
        OB_ASSERT((size % 4 == 0), "[VkCommandList] Size must be aligned to 4 bytes.");

        if (m_CurrentGraphicsPipeline)
        {
            const VulkanGraphicsPipeline& vkPipeline = *api_cast<const VulkanGraphicsPipeline*>(m_CurrentGraphicsPipeline);
            vkCmdPushConstants(m_CommandBuffer, vkPipeline.GetVkPipelineLayout(), vkPipeline.GetPushConstantsStage(), static_cast<uint32_t>(dstOffset), static_cast<uint32_t>(size), static_cast<const uint8_t*>(memory) + srcOffset);
        }
        else
        {
            const VulkanComputePipeline& vkPipeline = *api_cast<const VulkanComputePipeline*>(m_CurrentComputePipeline);
            vkCmdPushConstants(m_CommandBuffer, vkPipeline.GetVkPipelineLayout(), vkPipeline.GetPushConstantsStage(), static_cast<uint32_t>(dstOffset), static_cast<uint32_t>(size), static_cast<const uint8_t*>(memory) + srcOffset);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanCommandList::SetWaitStage(VkPipelineStageFlags2 waitStage)
    {
        VkPipelineStageFlags2 firstStage = GetFirstPipelineStage(waitStage);
        if (GetFirstPipelineStage(firstStage) < GetFirstPipelineStage(m_WaitStage))
            m_WaitStage = firstStage;
    }

}