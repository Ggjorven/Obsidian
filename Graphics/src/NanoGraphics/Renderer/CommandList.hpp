#pragma once
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"
#include "NanoGraphics/Renderer/CommandListSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanCommandList.hpp"
#include "NanoGraphics/Platform/Dummy/DummyCommandList.hpp"

#include <Nano/Nano.hpp>

#include <span>

namespace Nano::Graphics
{

    class Swapchain;
    class Image;
    class Buffer;
    class CommandListPool;

    ////////////////////////////////////////////////////////////////////////////////////
    // CommandList
    ////////////////////////////////////////////////////////////////////////////////////
    class CommandList
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanCommandList>,
            Types::EnumToType<Information::Structs::RenderingAPI::D3D12, Internal::DummyCommandList>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyCommandList>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyCommandList>
        >;
    public:
        // Destructor
        ~CommandList() = default;

        // Methods
        inline void Reset() const { m_CommandList.Reset(); }

        inline void ResetAndOpen() { m_CommandList.ResetAndOpen(); }
        inline void Open() { m_CommandList.Open(); }
        inline void Close() { m_CommandList.Close(); }

        inline void Submit(const CommandListSubmitArgs& args) const { return m_CommandList.Submit(args); }

        inline void WaitTillComplete() const { m_CommandList.WaitTillComplete(); }

        inline void CommitBarriers() { m_CommandList.CommitBarriers(); }

        // Object methods
        inline void StartTracking(const Image& image, ImageSubresourceSpecification subresources, ResourceState currentState = ResourceState::Unknown) { m_CommandList.StartTracking(image, subresources, currentState); }
        inline void StartTracking(const StagingImage& image, ResourceState currentState = ResourceState::Unknown) { m_CommandList.StartTracking(image, currentState); }
        inline void StartTracking(const Buffer& buffer, ResourceState currentState = ResourceState::Unknown) { m_CommandList.StartTracking(buffer, currentState); }

        inline void SetGraphicsState(const GraphicsState& state) { m_CommandList.SetGraphicsState(state); }

        inline void SetViewport(const Viewport& viewport) const { m_CommandList.SetViewport(viewport); }
        inline void SetScissor(const ScissorRect& scissor) const { m_CommandList.SetScissor(scissor); }

        inline void BindVertexBuffer(const Buffer& buffer) const { m_CommandList.BindVertexBuffer(buffer); }
        inline void BindIndexBuffer(const Buffer& buffer) const { m_CommandList.BindIndexBuffer(buffer); }

        inline void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, Image& src, const ImageSliceSpecification& srcSlice) { m_CommandList.CopyImage(dst, dstSlice, src, srcSlice); }
        inline void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, StagingImage& src, const ImageSliceSpecification& srcSlice) { m_CommandList.CopyImage(dst, dstSlice, src, srcSlice); }
        inline void CopyBuffer(Buffer& dst, Buffer& src, size_t size, size_t srcOffset = 0, size_t dstOffset = 0) { m_CommandList.CopyBuffer(dst, src, size, srcOffset, dstOffset); }

        // Draw methods
        inline void DrawIndexed(const DrawArguments& args) const { m_CommandList.DrawIndexed(args); }

        // Getters
        inline ResourceState GetResourceState(const Image& image, const ImageSubresourceSpecification& subresource) const { return m_CommandList.GetResourceState(image, subresource); }
        inline ResourceState GetResourceState(const Buffer& buffer) const { return m_CommandList.GetResourceState(buffer); }

    private:
        // Constructor
        inline CommandList(CommandListPool& pool, const CommandListSpecification& specs)
            : m_CommandList(pool, specs) {}

    private:
        Type m_CommandList;

        friend class CommandListPool;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // CommandListPool
    ////////////////////////////////////////////////////////////////////////////////////
    class CommandListPool : public Traits::NoCopy
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanCommandListPool>,
            Types::EnumToType<Information::Structs::RenderingAPI::D3D12, Internal::DummyCommandListPool>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyCommandListPool>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyCommandListPool>
        >;
    public:
        // Destructor
        ~CommandListPool() = default;

        // Creation methods // Note: Copy elision (RVO/NRVO) ensures object is constructed directly in the caller's stack frame.
        inline CommandList AllocateList(const CommandListSpecification& specs) { return CommandList(*this, specs); }
        inline void FreeList(CommandList& list) const { m_Pool.FreeList(list); }
        inline void FreeLists(std::span<CommandList*> lists) const { m_Pool.FreeLists(lists); }

        // Helper methods
        inline void ResetList(CommandList& list) const { m_Pool.ResetList(list); }
        inline void ResetAll() const { m_Pool.ResetAll(); }

    private:
        // Constructor
        inline CommandListPool(Swapchain& swapchain, const CommandListPoolSpecification& specs)
            : m_Pool(swapchain, specs) {}

    private:
        Type m_Pool;

        friend class Swapchain;
    };

}