#pragma once

#include "Obsidian/Core/Logging.hpp"

#include "Obsidian/Renderer/ImageSpec.hpp"
#include "Obsidian/Renderer/BufferSpec.hpp"

namespace Obsidian
{
    class Device;
    class Image;
    class Buffer;
    class CommandList;
}

namespace Obsidian::Internal
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
        void StartTracking(const Image& image, const ImageSubresourceSpecification& subresources, ResourceState currentState) const;
        void StartTracking(const Buffer& buffer, ResourceState currentState) const;
        void StopTracking(const Image& image);
        void StopTracking(const Buffer& buffer);

        void RequireImageState(const CommandList& list, Image& image, const ImageSubresourceSpecification& subresources, ResourceState state) const;
        void RequireBufferState(const CommandList& list, Buffer& buffer, ResourceState state) const;

        void ResolvePermanentState(const CommandList& list, Image& image, const ImageSubresourceSpecification& subresource) const;
        void ResolvePermanentState(const CommandList& list, Buffer& buffer) const;

        // Getters
        inline bool Contains(const Image& image) const { return m_ImageStates.contains(&image); }
        inline bool Contains(const Buffer& buffer) const { return m_BufferStates.contains(&buffer); }

        inline ImageState& GetImageState(const Image& image) const { return m_ImageStates[&image]; }
        inline BufferState& GetBufferState(const Buffer& buffer) const { return m_BufferStates[&buffer]; }

        ResourceState GetResourceState(const Image& image, const ImageSubresourceSpecification& subresource) const;
        ResourceState GetResourceState(const Buffer& buffer) const;

        inline std::vector<ImageBarrier>& GetImageBarriers(const CommandList& list) const { return m_ImageBarriers[&list]; }
        inline std::vector<BufferBarrier>& GetBufferBarriers(const CommandList& list) const { return m_BufferBarriers[&list]; }

    private:
        const Device& m_Device;

        mutable std::unordered_map<const Image*, ImageState> m_ImageStates = { };
        mutable std::unordered_map<const Buffer*, BufferState> m_BufferStates = { };

        mutable std::unordered_map<const CommandList*, std::vector<ImageBarrier>> m_ImageBarriers = { };
        mutable std::unordered_map<const CommandList*, std::vector<BufferBarrier>> m_BufferBarriers = { };
    };

}