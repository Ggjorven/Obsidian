#pragma once

#include "NanoGraphics/Renderer/ImageSpec.hpp"
#include "NanoGraphics/Renderer/BufferSpec.hpp"

namespace Nano::Graphics
{
    class Device;
    class Image;
    class Buffer;
}

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Barriers
    ////////////////////////////////////////////////////////////////////////////////////
    struct ImageBarrier
    {
    public:
        Image* ImagePtr = nullptr;

        MipLevel ImageMipLevel = 0;
        ArraySlice ImageArraySlice = 0;
        bool EntireTexture = false;

        ResourceState StateBefore = ResourceState::Unknown;
        ResourceState StateAfter = ResourceState::Unknown;
    };

    struct BufferBarrier
    {
    public:
        Buffer* BufferPtr = nullptr;

        ResourceState StateBefore = ResourceState::Unknown;
        ResourceState StateAfter = ResourceState::Unknown;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // States
    ////////////////////////////////////////////////////////////////////////////////////
    struct ImageState
    {
    public:
        std::vector<ResourceState> SubresourceStates = { };
        ResourceState State = ResourceState::Unknown;

        bool EnableUavBarriers = true; // Note: Just to keep track of the fact that the specification specified it
        bool FirstUavBarrierPlaced = false;
        bool PermanentTransition = false;
    };

    struct BufferState
    {
    public:
        ResourceState State = ResourceState::Unknown;

        bool EnableUavBarriers = true; // Note: Just to keep track of the fact that the specification specified it
        bool FirstUavBarrierPlaced = false;
        bool PermanentTransition = false;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // StateTracker
    ////////////////////////////////////////////////////////////////////////////////////
    class StateTracker
    {
    public:
        // Constructor & Destructor
        StateTracker(const Device& device);
        ~StateTracker();

        // Methods
        void Clear() const;

        void StartTracking(const Image& image, ImageSubresourceSpecification subresources, ResourceState currentState) const;
        void StartTracking(const Buffer& buffer, ResourceState currentState) const;
        void StopTracking(const Image& image);
        void StopTracking(const Buffer& buffer);

        void RequireImageState(Image& image, ImageSubresourceSpecification subresources, ResourceState state) const;
        void RequireBufferState(Buffer& buffer, ResourceState state) const;

        void ResolvePermanentState(Image& image, const ImageSubresourceSpecification& subresource) const;
        void ResolvePermanentState(Buffer& buffer) const;

        // Getters
        inline bool Contains(const Image& image) const { return m_ImageStates.contains(&image); }
        inline bool Contains(const Buffer& buffer) const { return m_BufferStates.contains(&buffer); }

        inline ImageState& GetImageState(const Image& image) const { return m_ImageStates[&image]; }
        inline BufferState& GetBufferState(const Buffer& buffer) const { return m_BufferStates[&buffer]; }

        ResourceState GetResourceState(const Image& image, ImageSubresourceSpecification subresource) const;
        ResourceState GetResourceState(const Buffer& buffer) const;

        inline std::vector<ImageBarrier>& GetImageBarriers() const { return m_ImageBarriers; }
        inline std::vector<BufferBarrier>& GetBufferBarriers() const { return m_BufferBarriers; }

    private:
        const Device& m_Device;

        mutable std::unordered_map<const Image*, ImageState> m_ImageStates = { };
        mutable std::unordered_map<const Buffer*, BufferState> m_BufferStates = { };

        mutable std::vector<ImageBarrier> m_ImageBarriers = { };
        mutable std::vector<BufferBarrier> m_BufferBarriers = { };
    };

}