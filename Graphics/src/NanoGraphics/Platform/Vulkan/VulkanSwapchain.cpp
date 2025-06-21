#include "ngpch.h"
#include "VulkanImage.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Core/Window.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/Swapchain.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"

#if defined(NG_PLATFORM_DESKTOP)
    #define GLFW_INCLUDE_VULKAN
    #include <GLFW/glfw3.h>
#endif

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanSwapchain::VulkanSwapchain(const Device& device, const SwapchainSpecification& specs)
        : m_Device(*api_cast<const VulkanDevice*>(&device)), m_Specification(specs)
    {
        #if defined(NG_PLATFORM_DESKTOP)
            VK_VERIFY(glfwCreateWindowSurface(m_Device.GetContext().GetVkInstance(), static_cast<GLFWwindow*>(m_Specification.WindowTarget->GetNativeWindow()), VulkanAllocator::GetCallbacks(), &m_Surface));
        #endif
        
        // Resize utilities
        {
            VkCommandPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            poolInfo.queueFamilyIndex = m_Device.GetContext().GetVulkanPhysicalDevice().GetQueueFamilyIndices().QueueFamily;
            
            VK_VERIFY(vkCreateCommandPool(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &poolInfo, VulkanAllocator::GetCallbacks(), &m_ResizePool));

            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = m_ResizePool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;

            vkAllocateCommandBuffers(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &allocInfo, &m_ResizeCommand);
        }

        Resize(m_Specification.WindowTarget->GetSize().x, m_Specification.WindowTarget->GetSize().y, m_Specification.VSync, m_Specification.RequestedFormat, m_Specification.RequestedColourSpace);
    
        // Semaphores
        {
            VkSemaphoreCreateInfo semaphoreInfo = {};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            for (size_t i = 0; i < m_ImageAvailableSemaphores.size(); i++)
            {
                VK_VERIFY(vkCreateSemaphore(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &semaphoreInfo, VulkanAllocator::GetCallbacks(), &m_ImageAvailableSemaphores[i]));
            
                if constexpr (Information::Validation)
                {
                    if (!m_Specification.DebugName.empty())
                        m_Device.GetContext().SetDebugName(m_ImageAvailableSemaphores[i], VK_OBJECT_TYPE_SEMAPHORE, std::format("ImageAvailable Semaphore({0}) for: {1}", i, m_Specification.DebugName));
                }
            }

            m_SwapchainPresentableSemaphores.resize(m_Images.size());
            for (size_t i = 0; i < m_SwapchainPresentableSemaphores.size(); i++)
            {
                VK_VERIFY(vkCreateSemaphore(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &semaphoreInfo, VulkanAllocator::GetCallbacks(), &m_SwapchainPresentableSemaphores[i]));

                if constexpr (Information::Validation)
                {
                    if (!m_Specification.DebugName.empty())
                        m_Device.GetContext().SetDebugName(m_SwapchainPresentableSemaphores[i], VK_OBJECT_TYPE_SEMAPHORE, std::format("Presentable Semaphore({0}) for: {1}", i, m_Specification.DebugName));
                }
            }

            VkSemaphoreTypeCreateInfo timelineInfo = {};
            timelineInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
            timelineInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
            timelineInfo.initialValue = 0;

            semaphoreInfo.pNext = &timelineInfo;

            VK_VERIFY(vkCreateSemaphore(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &semaphoreInfo, VulkanAllocator::GetCallbacks(), &m_TimelineSemaphore));
        
            if constexpr (Information::Validation)
            {
                if (!m_Specification.DebugName.empty())
                    m_Device.GetContext().SetDebugName(m_TimelineSemaphore, VK_OBJECT_TYPE_SEMAPHORE, std::format("Timeline Semaphore for: {0}", m_Specification.DebugName));
            }
        }
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Destruction methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanSwapchain::FreePool(CommandListPool& pool) const
    {
        VkDevice device = m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice();
        VkCommandPool commandPool = api_cast<VulkanCommandListPool*>(&pool)->GetVkCommandPool();
        m_Device.GetContext().Destroy([device, commandPool]() mutable
        {
            vkDestroyCommandPool(device, commandPool, VulkanAllocator::GetCallbacks());
        });
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanSwapchain::Resize(uint32_t width, uint32_t height)
    {
        Resize(width, height, m_Specification.VSync, m_Specification.RequestedFormat, m_Specification.RequestedColourSpace);
    }

    void VulkanSwapchain::Resize(uint32_t width, uint32_t height, bool vsync, Format colourFormat, ColourSpace colourSpace)
    {
        NG_PROFILE("VkSwapchain::Resize()");
        if (width == 0 || height == 0) [[unlikely]]
            return;

        SwapchainSupportDetails details = SwapchainSupportDetails::Query(m_Surface, m_Device.GetContext().GetVulkanPhysicalDevice().GetVkPhysicalDevice());
        
        if ((colourFormat != m_Specification.RequestedFormat) || (colourSpace != m_Specification.RequestedColourSpace)) [[unlikely]]
            ResolveFormatAndColourSpace(details, colourFormat, colourSpace);

        // Update specification (Note: The rest gets updated by methods)
        m_Specification.VSync = vsync;

        VkExtent2D swapchainExtent = {};
        if (details.Capabilities.currentExtent.width == 0xFFFFFFFF) // When it's 0xFFFFFFFF we can decide ourselves.
        {
            swapchainExtent.width = width;
            swapchainExtent.height = height;
        }
        else
        {
            swapchainExtent = details.Capabilities.currentExtent;
        }

        VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        if (!vsync)
        {
            if constexpr (Information::Validation)
            {
#if defined(NG_PLATFORM_APPLE)
                m_Device.GetContext().Warn("[VkSwapchain] Having VSync off on apple platforms is not recommended, this may cause screen tearing.");
#endif
            }

            for (size_t i = 0; i < details.PresentModes.size(); i++)
            {
                if (details.PresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                    break;
                }
                if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (details.PresentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
                {
                    swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
                }
            }
        }

        uint32_t desiredNumberOfSwapchainImages = details.Capabilities.minImageCount + 1;
        if ((details.Capabilities.maxImageCount > 0) && (desiredNumberOfSwapchainImages > details.Capabilities.maxImageCount))
            desiredNumberOfSwapchainImages = details.Capabilities.maxImageCount; // Fall back to max image count if desired exceeds it.

        // Get current transform?
        VkSurfaceTransformFlagsKHR preTransform;
        if (details.Capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
            preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        else
            preTransform = details.Capabilities.currentTransform;

        // Note: Not all platforms support alpha opaque, so check for possible values
        VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        std::vector<VkCompositeAlphaFlagBitsKHR> possibleAlphaFlags = { VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR, VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR };
        for (const auto& flag : possibleAlphaFlags)
        {
            if (details.Capabilities.supportedCompositeAlpha & flag)
            {
                compositeAlpha = flag;
                break;
            }
        }

        VkSwapchainCreateInfoKHR swapchainCI = {};
        swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCI.pNext = nullptr;
        swapchainCI.surface = m_Surface;
        swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
        swapchainCI.imageFormat = FormatToVkFormat(m_Specification.RequestedFormat);
        swapchainCI.imageColorSpace = ColourSpaceToVkColorSpaceKHR(m_Specification.RequestedColourSpace);
        swapchainCI.imageExtent = { swapchainExtent.width, swapchainExtent.height };
        swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCI.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(preTransform);
        swapchainCI.imageArrayLayers = 1;
        swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCI.queueFamilyIndexCount = 0;
        swapchainCI.pQueueFamilyIndices = nullptr;
        swapchainCI.presentMode = swapchainPresentMode;
        swapchainCI.oldSwapchain = m_Swapchain;
        swapchainCI.clipped = VK_TRUE; // Note: Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
        swapchainCI.compositeAlpha = compositeAlpha; 

        VkDevice device = m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice();
        VkSwapchainKHR oldSwapchain = m_Swapchain;
        VK_VERIFY(vkCreateSwapchainKHR(device, &swapchainCI, VulkanAllocator::GetCallbacks(), &m_Swapchain));

        if (oldSwapchain)
            vkDestroySwapchainKHR(device, oldSwapchain, VulkanAllocator::GetCallbacks()); // Destroys old swapchain images

        if constexpr (Information::Validation)
        {
            if (!m_Specification.DebugName.empty())
                m_Device.GetContext().SetDebugName(m_Swapchain, VK_OBJECT_TYPE_SWAPCHAIN_KHR, std::string(m_Specification.DebugName));
        }

        uint32_t imageCount = 0;
        Nano::Memory::StaticVector<VkImage, Information::MaxImageCount> swapchainImages = { };

        VK_VERIFY(vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, nullptr));
        NG_ASSERT((imageCount <= Information::MaxImageCount), "[VkSwapchain] More images provided than we allow.");
        swapchainImages.resize(imageCount);
        m_Images.resize(imageCount);
        VK_VERIFY(vkGetSwapchainImagesKHR(device, m_Swapchain, &imageCount, swapchainImages.data()));

        for (uint32_t i = 0; i < imageCount; i++)
        {
            std::string debugName = std::format("Image({0}) for: {1}", i, m_Specification.DebugName);
            ImageSpecification imageSpec = ImageSpecification()
                .SetImageDimension(ImageDimension::Image2D)
                .SetImageFormat(colourFormat)
                .SetWidthAndHeight(width, height)
                .SetIsRenderTarget(true)
                .SetPermanentState(ResourceState::Present)
                .SetDebugName(debugName);

            VulkanImage& vkImage = *api_cast<VulkanImage*>(&m_Images[i].Get());

            if (m_Images[i].IsConstructed())
            {
                m_Device.DestroySubresourceViews(m_Images[i].Get());
                vkImage.SetInternalData(imageSpec, swapchainImages[i]);
            }
            else
            {
                new (m_Images[i].GetInternalBytes()) Image(*api_cast<const Device*>(&m_Device));
                vkImage.SetInternalData(imageSpec, swapchainImages[i]);
            }

            ImageSubresourceSpecification imageViewSpec = ImageSubresourceSpecification(0, ImageSubresourceSpecification::AllMipLevels, 0, ImageSubresourceSpecification::AllArraySlices);
            (void)vkImage.GetSubresourceView(imageViewSpec, ImageDimension::Image2D, colourFormat, 0, ImageSubresourceViewType::AllAspects); // Note: Makes sure to already lazy initialize the image view
        
            if constexpr (Information::Validation)
            {
                if (!m_Specification.DebugName.empty())
                    m_Device.GetContext().SetDebugName(swapchainImages[i], VK_OBJECT_TYPE_IMAGE, debugName);
            }
        }

        // Transition to PresentSrc
        {
            VK_VERIFY(vkResetCommandBuffer(m_ResizeCommand, 0));

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            VK_VERIFY(vkBeginCommandBuffer(m_ResizeCommand, &beginInfo));

            VkImageMemoryBarrier2 barrier2 = {};
            barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            barrier2.srcStageMask = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            barrier2.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            barrier2.srcAccessMask = VK_ACCESS_2_NONE;
            barrier2.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
            barrier2.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier2.subresourceRange.baseMipLevel = 0;
            barrier2.subresourceRange.levelCount = 1;
            barrier2.subresourceRange.baseArrayLayer = 0;
            barrier2.subresourceRange.layerCount = 1;

            // Set all the images
            Nano::Memory::StaticVector<VkImageMemoryBarrier2, Information::MaxImageCount> barriers = { };
            barriers.resize(m_Images.size());
            for (size_t i = 0; i < m_Images.size(); i++)
            {
                barriers[i] = barrier2;
                barriers[i].image = api_cast<VulkanImage*>(&m_Images[i].Get())->GetVkImage();
            }

            VkDependencyInfo dependencyInfo = {};
            dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(barriers.size());
            dependencyInfo.pImageMemoryBarriers = barriers.data();


#if defined(NG_PLATFORM_APPLE)
            VkExtension::g_vkCmdPipelineBarrier2KHR(m_ResizeCommand, &dependencyInfo);
#else
            vkCmdPipelineBarrier2(m_ResizeCommand, &dependencyInfo);
#endif

            VK_VERIFY(vkEndCommandBuffer(m_ResizeCommand));

            VkCommandBufferSubmitInfo commandBufferInfo = {};
            commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            commandBufferInfo.commandBuffer = m_ResizeCommand;

            VkSubmitInfo2 submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
            submitInfo.commandBufferInfoCount = 1;
            submitInfo.pCommandBufferInfos = &commandBufferInfo;
#if defined(NG_PLATFORM_APPLE)
            VkExtension::g_vkQueueSubmit2KHR(m_Device.GetContext().GetVulkanLogicalDevice().GetVkQueue(CommandQueue::Graphics), 1, &submitInfo, VK_NULL_HANDLE);
#else
            vkQueueSubmit2(m_Device.GetContext().GetVulkanLogicalDevice().GetVkQueue(CommandQueue::Graphics), 1, &submitInfo, VK_NULL_HANDLE);
#endif
            
            VK_VERIFY(vkQueueWaitIdle(m_Device.GetContext().GetVulkanLogicalDevice().GetVkQueue(CommandQueue::Graphics)));
        }
    }

    void VulkanSwapchain::AcquireNextImage()
    {
        NG_PROFILE("VkSwapchain::AcquireImage()");

        // Wait for this frame's previous last value
        {
            VkSemaphoreWaitInfo waitInfo = {};
            waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
            waitInfo.semaphoreCount = 1;
            waitInfo.pSemaphores = &m_TimelineSemaphore;
            waitInfo.pValues = &m_WaitTimelineValues[m_CurrentFrame];

            vkWaitSemaphores(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &waitInfo, std::numeric_limits<uint64_t>::max());
        }

        // Acquire image
        VkResult result = vkAcquireNextImageKHR(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), m_Swapchain, std::numeric_limits<uint64_t>::max(), m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_AcquiredImage);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            Resize(m_Specification.WindowTarget->GetSize().x, m_Specification.WindowTarget->GetSize().y);
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            m_Device.GetContext().Error("[VkSwapchain] Failed to acquire Swapchain image!");
        }
    }

    void VulkanSwapchain::Present()
    {
        NG_PROFILE("VkSwapchain::Present()");

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_SwapchainPresentableSemaphores[m_AcquiredImage];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &m_Swapchain;
        presentInfo.pImageIndices = &m_AcquiredImage;
        presentInfo.pResults = nullptr; // Optional

        VkResult result = VK_SUCCESS;
        {
            NG_PROFILE("VkSwapchain::Present::QueuePresent");
            result = vkQueuePresentKHR(m_Device.GetContext().GetVulkanLogicalDevice().GetVkQueue(CommandQueue::Present), &presentInfo);
        }

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            Resize(m_Specification.WindowTarget->GetSize().x, m_Specification.WindowTarget->GetSize().y);
        }
        else if (result != VK_SUCCESS)
        {
            m_Device.GetContext().Error("[VkSwapchain] Failed to present Swapchain image.");
        }

        m_WaitTimelineValues[m_CurrentFrame] = m_CurrentTimelineValue;
        m_CurrentFrame = (m_CurrentFrame + 1) % Information::FramesInFlight;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Internal methods
    ////////////////////////////////////////////////////////////////////////////////////
    uint64_t VulkanSwapchain::GetPreviousCommandListWaitValue(const VulkanCommandList& commandList) const
    {
        NG_ASSERT(m_CommandListSemaphoreValues.contains(&commandList), "[VkSwapchain] Commandlist is not known in current Swapchain.");
        return m_CommandListSemaphoreValues.at(&commandList);
    }

    uint64_t VulkanSwapchain::RetrieveCommandListWaitValue(const VulkanCommandList& commandList)
    {
        uint64_t value = ++m_CurrentTimelineValue;
        m_CommandListSemaphoreValues[&commandList] = value;
        return value;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanSwapchain::ResolveFormatAndColourSpace(const SwapchainSupportDetails& details, Format format, ColourSpace space)
    {
        const std::vector<VkSurfaceFormatKHR>& formats = details.Formats;
        
        // If the surface format list only includes one entry with VK_FORMAT_UNDEFINED anything is fine, so the requested can stay
        if ((formats.size() == 1ull) && (formats[0].format == VK_FORMAT_UNDEFINED))
        {
            m_Specification.RequestedFormat = format;
            m_Specification.RequestedColourSpace = space;
        }
        else
        {
            bool foundDesiredCombination = false;
            for (const auto& [form, colspace] : formats)
            {
                if ((form == FormatToVkFormat(format)) && (colspace == ColourSpaceToVkColorSpaceKHR(space)))
                {
                    m_Specification.RequestedFormat = format;
                    m_Specification.RequestedColourSpace = space;

                    foundDesiredCombination = true;
                    break;
                }
            }

            // If not available, try again
            if (!foundDesiredCombination && space == ColourSpace::SRGB)
            {
                m_Device.GetContext().Error("[VkSwapchain] Failed to resolve format and colourspace. Defaulting to BGRA8Unorm & SRGB.");
                ResolveFormatAndColourSpace(details, Format::BGRA8Unorm, ColourSpace::SRGB);
            }
            else if (!foundDesiredCombination)
                ResolveFormatAndColourSpace(details, format, ColourSpace::SRGB);
        }
    }

}