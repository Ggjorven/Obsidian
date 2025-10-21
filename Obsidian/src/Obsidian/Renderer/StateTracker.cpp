#include "obpch.h"
#include "StateTracker.hpp"

#include "Obsidian/Core/Logging.hpp"
#include "Obsidian/Core/Information.hpp"
#include "Obsidian/Utils/Profiler.hpp"

#include "Obsidian/Renderer/Device.hpp"
#include "Obsidian/Renderer/Image.hpp"
#include "Obsidian/Renderer/Buffer.hpp"

namespace Obsidian::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    StateTracker::StateTracker(const Device& device)
        : m_Device(device)
    {
    }

    StateTracker::~StateTracker()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void StateTracker::StartTracking(const Image& image, const ImageSubresourceSpecification& subresources, ResourceState currentState) const
    {
        OB_ASSERT((!Contains(image)), "[StateTracker] Started tracking an object that's already being tracked.");

        m_ImageStates.emplace();
        SetImageState(image, subresources, currentState);
    }

    void StateTracker::StartTracking(const Buffer& buffer, ResourceState currentState) const
    {
        OB_ASSERT((!Contains(buffer)), "[StateTracker] Started tracking an object that's already being tracked.");

        m_BufferStates.emplace();
        SetBufferState(buffer, currentState);
    }

    void StateTracker::StopTracking(const Image& image)
    {
        if (Contains(image))
            m_ImageStates.erase(&image);
    }

    void StateTracker::StopTracking(const Buffer& buffer)
    {
        if (Contains(buffer))
            m_BufferStates.erase(&buffer);
    }

    void StateTracker::RequireImageState(const CommandList& list, Image& image, const ImageSubresourceSpecification& subresources, ResourceState state) const
    {
        OB_ASSERT(Contains(image), "[StateTracker] Using an untracked image is not allowed, call StartTracking() on image.");

        const ImageSpecification& imageSpec = image.GetSpecification();
        ImageSubresourceSpecification resSubresources = ResolveImageSubresource(subresources, imageSpec, false);

        ImageState& currentState = m_ImageStates[&image];

        if (resSubresources.IsEntireTexture(image.GetSpecification())) // Entire texture
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

                m_ImageBarriers[&list].push_back(barrier);
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
                if constexpr (Information::Validation)
                {
                    if (currentState.State == ResourceState::Unknown)
                        OB_LOG_ERROR("[StateTracker] No previous subresource state was set and currenstate is Unknown. This is not allowed.");
                        //m_Device.GetContext().Error("[StateTracker] No previous subresource state was set and currenstate is Unknown. This is not allowed.");
                }

                currentState.SubresourceStates.resize(static_cast<size_t>(imageSpec.MipLevels) * imageSpec.ArraySize, currentState.State);
                currentState.State = ResourceState::Unknown;
                stateExpanded = true;
            }

            bool anyUavBarrier = false;
            for (ArraySlice arraySlice = resSubresources.BaseArraySlice; arraySlice < resSubresources.BaseArraySlice + resSubresources.NumArraySlices; arraySlice++)
            {
                for (MipLevel mipLevel = resSubresources.BaseMipLevel; mipLevel < resSubresources.BaseMipLevel + resSubresources.NumMipLevels; mipLevel++)
                {
                    size_t subresourceIndex = ImageSubresourceSpecification::SubresourceIndex(mipLevel, arraySlice, imageSpec);
                    auto priorState = currentState.SubresourceStates[subresourceIndex];

                    if constexpr (Information::Validation)
                    {
                        if (priorState == ResourceState::Unknown && !stateExpanded)
                            OB_LOG_ERROR("[StateTracker] Subresource state was set to Unknown. This is not allowed.");
                            //m_Device.GetContext().Error("[StateTracker] Subresource state was set to Unknown. This is not allowed.");
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

                        m_ImageBarriers[&list].push_back(barrier);
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

    void StateTracker::RequireBufferState(const CommandList& list, Buffer& buffer, ResourceState state) const
    {
        OB_ASSERT(Contains(buffer), "[StateTracker] Using an untracked buffer is not allowed, call StartTracking() on buffer.");

        BufferState& currentState = m_BufferStates[&buffer];

        bool transitionNecessary = (currentState.State != state);
        bool uavNecessary = (static_cast<bool>((state & ResourceState::UnorderedAccess)) != false) && (currentState.EnableUavBarriers || !currentState.FirstUavBarrierPlaced);

        if (transitionNecessary)
        {
            for (BufferBarrier& barrier : m_BufferBarriers[&list]) // Check if the buffer isn't already begin transitioned and add the flag to the after state.
            {
                if (barrier.BufferPtr == &buffer)
                {
                    barrier.StateAfter |= state;
                    currentState.State = barrier.StateAfter;
                    return; // Early return
                }
            }
        }

        if (transitionNecessary || uavNecessary)
        {
            BufferBarrier barrier;
            barrier.BufferPtr = &buffer;
            barrier.StateBefore = currentState.State;
            barrier.StateAfter = state;
            m_BufferBarriers[&list].push_back(barrier);
        }

        if (uavNecessary && !transitionNecessary)
            currentState.FirstUavBarrierPlaced = true;

        currentState.State = state;
    }

    void StateTracker::ResolvePermanentState(const CommandList& list, Image& image, const ImageSubresourceSpecification& subresource) const
    {
        OB_ASSERT((Contains(image)), "[StateTracker] Cannot get resourcestate for an untracked object.");
        OB_ASSERT(((subresource.NumMipLevels == 1) && (subresource.NumArraySlices == 1)), "[StateTracker] Cannot get a single ResourceState from multiple subresources.");

        if (!image.GetSpecification().HasPermanentState())
            return;

        ResourceState state = image.GetSpecification().PermanentState;
        ResourceState currentState = GetResourceState(image, subresource);

        if (state != currentState)
            RequireImageState(list, image, subresource, state);
    }

    void StateTracker::ResolvePermanentState(const CommandList& list, Buffer& buffer) const
    {
        OB_ASSERT((Contains(buffer)), "[StateTracker] Cannot get resourcestate for an untracked object.");
        if (!buffer.GetSpecification().HasPermanentState())
            return;

        ResourceState state = buffer.GetSpecification().PermanentState;
        ResourceState currentState = GetResourceState(buffer);

        if (state != currentState)
            RequireBufferState(list, buffer, state);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Getters
    ////////////////////////////////////////////////////////////////////////////////////
    ResourceState StateTracker::GetResourceState(const Image& image, const ImageSubresourceSpecification& subresource) const
    {
        OB_ASSERT((Contains(image)), "[StateTracker] Cannot get resourcestate for an untracked object.");
        
        const ImageSpecification& imageSpec = image.GetSpecification();
        ImageSubresourceSpecification resSubresources = ResolveImageSubresource(subresource, imageSpec, false);
        
        OB_ASSERT(((resSubresources.NumMipLevels == 1) && (resSubresources.NumArraySlices == 1)), "[StateTracker] Cannot get a single ResourceState from multiple subresources.");

        if (subresource.IsEntireTexture(imageSpec))
            return m_ImageStates.at(&image).State;

        return m_ImageStates.at(&image).SubresourceStates[ImageSubresourceSpecification::SubresourceIndex(subresource.BaseMipLevel, subresource.BaseArraySlice, imageSpec)];
    }

    ResourceState StateTracker::GetResourceState(const Buffer& buffer) const
    {
        OB_ASSERT((Contains(buffer)), "[StateTracker] Cannot get resourcestate for an untracked object.");
        return m_BufferStates.at(&buffer).State;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Setters
    ////////////////////////////////////////////////////////////////////////////////////
    void StateTracker::SetImageState(const Image& image, const ImageSubresourceSpecification& subresources, ResourceState state) const
    {
        ImageState& currentState = m_ImageStates[&image];
        const ImageSpecification& imageSpec = image.GetSpecification();
        ImageSubresourceSpecification resSubresources = ResolveImageSubresource(subresources, imageSpec, false);

        if (resSubresources.IsEntireTexture(imageSpec))
        {
            currentState.State = state;
        }
        else
        {
            currentState.SubresourceStates.resize(static_cast<size_t>(imageSpec.MipLevels) * imageSpec.ArraySize, currentState.State);
            currentState.State = ResourceState::Unknown;

            for (MipLevel mipLevel = resSubresources.BaseMipLevel; mipLevel < resSubresources.BaseMipLevel + resSubresources.NumMipLevels; mipLevel++)
            {
                for (ArraySlice arraySlice = resSubresources.BaseArraySlice; arraySlice < resSubresources.BaseArraySlice + resSubresources.NumArraySlices; arraySlice++)
                    currentState.SubresourceStates[ImageSubresourceSpecification::SubresourceIndex(mipLevel, arraySlice, imageSpec)] = state;
            }
        }
    }

    void StateTracker::SetBufferState(const Buffer& buffer, ResourceState state) const
    {
        BufferState& currentState = m_BufferStates[&buffer];
        currentState.State = state;
    }

}