#include "ngpch.h"
#include "VulkanCommandList.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/CommandList.hpp"
#include "NanoGraphics/Renderer/Swapchain.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"

namespace Nano::Graphics::Internal
{

    static_assert(std::is_same_v<Device::Type, VulkanDevice>, "Current Device::Type is not VulkanDevice and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<Swapchain::Type, VulkanSwapchain>, "Current Swapchain::Type is not VulkanSwapchain and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<CommandList::Type, VulkanCommandList>, "Current CommandList::Type is not VulkanCommandList and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<CommandListPool::Type, VulkanCommandListPool>, "Current CommandListPool::Type is not VulkanImage and Vulkan source code is being compiled.");

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanCommandListPool::VulkanCommandListPool(Swapchain& swapchain, const CommandListPoolSpecification& specs)
        : m_Swapchain(*reinterpret_cast<VulkanSwapchain*>(&swapchain)), m_Specification(specs)
    {
        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Note: Allows us to reset the command buffer and reuse it.
        poolInfo.queueFamilyIndex = m_Swapchain.GetVulkanDevice().GetContext().GetVulkanPhysicalDevice().GetQueueFamilyIndices().QueueFamily;
        
        VK_VERIFY(vkCreateCommandPool(m_Swapchain.GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkDevice(), &poolInfo, VulkanAllocator::GetCallbacks(), &m_CommandPool));

        if constexpr (VulkanContext::Validation)
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
        VkCommandBuffer commandBuffer = (*reinterpret_cast<VulkanCommandList*>(&list)).GetVkCommandBuffer();
        m_Swapchain.GetVulkanDevice().GetContext().Destroy([device, commandPool = m_CommandPool, commandBuffer]() mutable
        { 
            vkFreeCommandBuffers(device, commandPool, 1ul, &commandBuffer);
        });
    }

    void VulkanCommandListPool::FreeLists(std::span<CommandList*> lists) const
    {
        std::vector<VkCommandBuffer> commandBuffers;
        commandBuffers.reserve(lists.size());

        for (auto& list : lists)
        {
            VkCommandBuffer commandBuffer = (*reinterpret_cast<VulkanCommandList*>(&list)).GetVkCommandBuffer();
            commandBuffers.push_back(commandBuffer);
        }

        VkDevice device = m_Swapchain.GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkDevice();
        m_Swapchain.GetVulkanDevice().GetContext().Destroy([device, commandPool = m_CommandPool, commandBuffers = std::move(commandBuffers)]() mutable
        {
            vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        });
    }

    void VulkanCommandListPool::ResetList(CommandList& list) const
    {
        vkResetCommandBuffer((*reinterpret_cast<VulkanCommandList*>(&list)).GetVkCommandBuffer(), 0);
    }

    void VulkanCommandListPool::ResetAll() const
    {
        vkResetCommandPool(m_Swapchain.GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkDevice(), m_CommandPool, 0);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanCommandList::VulkanCommandList(CommandListPool& pool, const CommandListSpecification& specs)
        : m_Pool(*reinterpret_cast<VulkanCommandListPool*>(&pool)), m_Specification(specs), m_StateTracker(m_Pool.GetVulkanSwapchain().GetVulkanDevice())
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_Pool.GetVkCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VK_VERIFY(vkAllocateCommandBuffers(m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkDevice(), &allocInfo, &m_CommandBuffer));

        if constexpr (VulkanContext::Validation)
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
    void VulkanCommandList::Reset() const
    {
        vkResetCommandBuffer(m_CommandBuffer, 0);
    }

    void VulkanCommandList::ResetAndOpen()
    {
        Reset();
        Open();
    }

    void VulkanCommandList::Open()
    {
        NG_PROFILE("VulkanCommandList::Open()");
        m_WaitStage = VK_PIPELINE_STAGE_2_NONE;

        {
            NG_PROFILE("VulkanCommandList::Open::Begin");
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            VK_VERIFY(vkBeginCommandBuffer(m_CommandBuffer, &beginInfo));
        }
    }

    void VulkanCommandList::Close()
    {
        NG_PROFILE("VulkanCommandList::Close()");

        // Renderpass
        {
            VkSubpassEndInfo endInfo = {};
            endInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO;

            vkCmdEndRenderPass2(m_CommandBuffer, &endInfo);
        }

        m_GraphicsState = GraphicsState();

        {
            NG_PROFILE("VulkanCommandList::Close::End");
            VK_VERIFY(vkEndCommandBuffer(m_CommandBuffer));
        }
    }

    void VulkanCommandList::Submit(const CommandListSubmitArgs& args) const 
    {
        NG_PROFILE("VulkanCommandBuffer::Submit()");

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
            info.value = swapchain.GetPreviousCommandListWaitValue(*reinterpret_cast<const VulkanCommandList*>(list));
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
            info.semaphore = swapchain.GetVkSwapchainPresentableSemaphore(swapchain.GetCurrentFrame());
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
        
        VK_VERIFY(vkQueueSubmit2(m_Pool.GetVulkanSwapchain().GetVulkanDevice().GetContext().GetVulkanLogicalDevice().GetVkQueue(args.Queue), 1, &submitInfo, nullptr));
    }

    void VulkanCommandList::CommitBarriers()
    {
        m_StateTracker.CommitBarriers(m_CommandBuffer);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Object methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanCommandList::StartTracking(const Image& image, ImageSubresourceSpecification subresources, ResourceState currentState)
    {
        m_StateTracker.StartTracking(image, subresources, currentState);
    }

    void VulkanCommandList::SetGraphicsState(const GraphicsState& state)
    {
        m_GraphicsState = state;

        //NG_ASSERT(m_GraphicsState.Pipeline, "[VkCommandList] No pipeline passed in.");
        NG_ASSERT(m_GraphicsState.Pass, "[VkCommandList] No Renderpass passed in.");

        // Renderpass
        {
            VulkanRenderpass& renderpass = *reinterpret_cast<VulkanRenderpass*>(state.Pass);
            VulkanFramebuffer* framebuffer = nullptr;
            if (state.Frame)
                framebuffer = reinterpret_cast<VulkanFramebuffer*>(state.Frame);
            else
                framebuffer = reinterpret_cast<VulkanFramebuffer*>(&renderpass.GetFramebuffer(static_cast<uint8_t>(m_Pool.GetVulkanSwapchain().GetCurrentFrame())));
        
            VkRenderPassBeginInfo renderpassInfo = {};
            renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderpassInfo.renderPass = renderpass.GetVkRenderPass();
            renderpassInfo.framebuffer = framebuffer->GetVkFramebuffer();
            renderpassInfo.renderArea.offset = { 0, 0 };
            renderpassInfo.renderArea.extent = { static_cast<uint32_t>(state.ViewportState.GetWidth()), static_cast<uint32_t>(state.ViewportState.GetHeight()) };

            // Clear values
            Nano::Memory::StaticVector<VkClearValue, 2> clearValues;
            if (framebuffer->GetSpecification().ColourAttachment.IsValid())
                clearValues.push_back(VkClearValue({ state.ColourClear.r, state.ColourClear.g, state.ColourClear.b, state.ColourClear.a }));
            if (framebuffer->GetSpecification().DepthAttachment.IsValid())
                clearValues.push_back(VkClearValue({ state.DepthClear, 0 }));

            renderpassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderpassInfo.pClearValues = clearValues.data();

            VkSubpassBeginInfo subpassInfo = {};
            subpassInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO;
            subpassInfo.contents = VK_SUBPASS_CONTENTS_INLINE;

            vkCmdBeginRenderPass2(m_CommandBuffer, &renderpassInfo, &subpassInfo);
        }

        SetViewport(state.ViewportState);
        SetScissor(state.Scissor);
    }

    void VulkanCommandList::SetViewport(const Viewport& viewport) const
    {
        // Note: For future DX coords?
        //VkViewport(v.minX, v.maxY, v.maxX - v.minX, -(v.maxY - v.minY), v.minZ, v.maxZ);

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
        VkRect2D vkScissor = {};
        vkScissor.offset = { scissor.MinX, scissor.MinY };
        vkScissor.extent = { static_cast<uint32_t>(scissor.GetWidth()), static_cast<uint32_t>(scissor.GetHeight()) };
        vkCmdSetScissor(m_CommandBuffer, 0, 1, &vkScissor);
    }

    void VulkanCommandList::CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, Image& src, const ImageSliceSpecification& srcSlice)
    {
        NG_ASSERT(m_StateTracker.Contains(dst), "[VkCommandList] Using an untracked image is not allowed, call StartTracking() on dst image.");
        NG_ASSERT(m_StateTracker.Contains(src), "[VkCommandList] Using an untracked image is not allowed, call StartTracking() on src image.");

        SetWaitStage(VK_PIPELINE_STAGE_2_TRANSFER_BIT);

        VulkanImage& vulkanDst = *reinterpret_cast<VulkanImage*>(&dst);
        VulkanImage& vulkanSrc = *reinterpret_cast<VulkanImage*>(&src);

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

        VkFormat dstFormat = FormatToVkFormat(dst.GetSpecification().ImageFormat);
        VkImageAspectFlags dstAspectFlags = GuessSubresourceImageAspectFlags(dstFormat, ImageSubresourceViewType::AllAspects);

        VkExtent3D extent = VkExtent3D(
            std::min(resSrcSlice.Width, resDstSlice.Width),
            std::min(resSrcSlice.Height, resDstSlice.Height),
            std::min(resSrcSlice.Depth, resDstSlice.Depth)
        );

        m_StateTracker.RequireImageState(src, ImageSubresourceSpecification(resSrcSlice.ImageMipLevel, 1, resSrcSlice.ImageArraySlice, 1), ResourceState::CopySrc);
        m_StateTracker.RequireImageState(dst, ImageSubresourceSpecification(resDstSlice.ImageMipLevel, 1, resDstSlice.ImageArraySlice, 1), ResourceState::CopyDst);
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

        vkCmdCopyImage2(m_CommandBuffer, &copyInfo);
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