#include "ngpch.h"
#include "VulkanStateTracker.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Image.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"

namespace Nano::Graphics::Internal
{

    static_assert(std::is_same_v<Image::Type, VulkanImage>, "Current Image::Type is not VulkanImage and Vulkan source code is being compiled.");

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanStateTracker::VulkanStateTracker(const VulkanDevice& device)
        : m_Device(device)
    {
    }

    VulkanStateTracker::~VulkanStateTracker()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanStateTracker::Clear()
    {
        m_ImageStates.clear();
    }

    void VulkanStateTracker::StartTracking(const Image& image, ImageSubresourceSpecification subresources, ResourceState currentState)
    {
        NG_ASSERT(!(m_ImageStates.contains(&image)), "[VkStateTracker] Started tracking an object that's already being tracked.");
        
        m_ImageStates.emplace();
        VulkanImageState& state = m_ImageStates[&image];
        const ImageSpecification& imageSpec = image.GetSpecification();
        subresources = ResolveImageSubresouce(subresources, imageSpec, false);

        if (subresources.IsEntireTexture(imageSpec))
        {
            state.State = currentState;
        }
        else
        {
            state.SubresourceStates.resize(static_cast<size_t>(imageSpec.MipLevels) * imageSpec.ArraySize, state.State);
            state.State = ResourceState::Unknown;

            for (MipLevel mipLevel = subresources.BaseMipLevel; mipLevel < subresources.BaseMipLevel + subresources.NumMipLevels; mipLevel++)
            {
                for (ArraySlice arraySlice = subresources.BaseArraySlice; arraySlice < subresources.BaseArraySlice + subresources.NumArraySlices; arraySlice++)
                    state.SubresourceStates[ImageSubresourceSpecification::SubresourceIndex(mipLevel, arraySlice, imageSpec)] = currentState;
            }
        }
    }

    void VulkanStateTracker::RequireImageState(Image& image, ImageSubresourceSpecification subresources, ResourceState state)
    {
        NG_ASSERT(Contains(image), "[VkStateTracker] Using an untracked image is not allowed, call StartTracking() on image.");

        const ImageSpecification& imageSpec = image.GetSpecification();
        subresources = ResolveImageSubresouce(subresources, image.GetSpecification(), false);

        VulkanImageState& currentState = m_ImageStates[&image];

        if (subresources.IsEntireTexture(image.GetSpecification())) // Entire texture
        {
            bool transitionNecessary = (currentState.State != state);
            bool uavNecessary = (static_cast<bool>((state & ResourceState::UnorderedAccess)) != false) && (currentState.EnableUavBarriers || !currentState.FirstUavBarrierPlaced);

            if (transitionNecessary || uavNecessary)
            {
                ImageBarrier barrier = {};
                barrier.ImagePtr = &image;
                barrier.EntireTexture = true;
                barrier.StateBefore = currentState.State;
                barrier.StateAfter = state;

                m_ImageBarriers.push_back(barrier);
            }

            currentState.State = state;

            if (uavNecessary && !transitionNecessary)
                currentState.FirstUavBarrierPlaced = true;
        }
        else // Convert all subresources
        {
            bool stateExpanded = false;
            if (currentState.SubresourceStates.empty()) // If we don't have any knowledge of previous subresource states
            {
                if constexpr (VulkanContext::Validation)
                {
                    if (currentState.State == ResourceState::Unknown)
                        m_Device.GetContext().Error("[VkStateTracker] No previous subresource state was set and currenstate is Unknown. This is not allowed.");
                }

                currentState.SubresourceStates.resize(static_cast<size_t>(imageSpec.MipLevels) * imageSpec.ArraySize, currentState.State);
                currentState.State = ResourceState::Unknown;
                stateExpanded = true;
            }

            bool anyUavBarrier = false;
            for (ArraySlice arraySlice = subresources.BaseArraySlice; arraySlice < subresources.BaseArraySlice + subresources.NumArraySlices; arraySlice++)
            {
                for (MipLevel mipLevel = subresources.BaseMipLevel; mipLevel < subresources.BaseMipLevel + subresources.NumMipLevels; mipLevel++)
                {
                    size_t subresourceIndex = ImageSubresourceSpecification::SubresourceIndex(mipLevel, arraySlice, imageSpec);
                    auto priorState = currentState.SubresourceStates[subresourceIndex];

                    if constexpr (VulkanContext::Validation)
                    {
                        if (priorState == ResourceState::Unknown && !stateExpanded)
                            m_Device.GetContext().Error("[VkStateTracker] Subresource state was set to Unknown. This is not allowed.");
                    }

                    bool transitionNecessary = (priorState != state);
                    bool uavNecessary = (static_cast<bool>((state & ResourceState::UnorderedAccess)) != false) && !anyUavBarrier && (currentState.EnableUavBarriers || !currentState.FirstUavBarrierPlaced);

                    if (transitionNecessary || uavNecessary)
                    {
                        ImageBarrier barrier;
                        barrier.ImagePtr = &image;
                        barrier.EntireTexture = false;
                        barrier.ImageMipLevel = mipLevel;
                        barrier.ImageArraySlice = arraySlice;
                        barrier.StateBefore = priorState;
                        barrier.StateAfter = state;

                        m_ImageBarriers.push_back(barrier);
                    }

                    currentState.SubresourceStates[subresourceIndex] = state;

                    if (uavNecessary && !transitionNecessary)
                    {
                        anyUavBarrier = true;
                        currentState.FirstUavBarrierPlaced = true;
                    }
                }
            }
        }
    }

    void VulkanStateTracker::CommitBarriers(VkCommandBuffer cmdBuf)
    {
        if (m_ImageBarriers.empty())
            return;

        std::vector<VkImageMemoryBarrier2> imageBarriers;
        imageBarriers.reserve(m_ImageBarriers.size());

        for (const ImageBarrier& imageBarrier : m_ImageBarriers)
        {
            const ResourceStateMapping& before = ResourceStateToMapping(imageBarrier.StateBefore);
            const ResourceStateMapping& after = ResourceStateToMapping(imageBarrier.StateAfter);

            NG_ASSERT((after.ImageLayout != VK_IMAGE_LAYOUT_UNDEFINED), "[VkCommandList] Can't transition to undefined layout.");

            Image& image = *imageBarrier.ImagePtr;
            VulkanImage& vulkanImage = *reinterpret_cast<VulkanImage*>(imageBarrier.ImagePtr);

            VkImageMemoryBarrier2 barrier2 = {};
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

            imageBarriers.push_back(barrier2);
        }

        if (!imageBarriers.empty())
        {
            VkDependencyInfo dependencyInfo = {};
            dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
            dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(imageBarriers.size());
            dependencyInfo.pImageMemoryBarriers = imageBarriers.data();

            vkCmdPipelineBarrier2(cmdBuf, &dependencyInfo);
        }

        m_ImageBarriers.clear();

        // TODO: Buffer barriers
    }


}