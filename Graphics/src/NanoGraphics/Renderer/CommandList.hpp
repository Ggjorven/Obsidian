#pragma once
#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"
#include "NanoGraphics/Renderer/CommandListSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanCommandList.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12CommandList.hpp"
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
            Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12CommandList>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyCommandList>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyCommandList>
        >;
    public:
        // Destructor
        ~CommandList() = default;

        // Methods
        inline void Open() { m_Impl->Open(); }
        inline void Close() { m_Impl->Close(); }

        inline void Submit(const CommandListSubmitArgs& args) const { return m_Impl->Submit(args); }

        inline void WaitTillComplete() const { m_Impl->WaitTillComplete(); }

        inline void CommitBarriers() { m_Impl->CommitBarriers(); }

        // Object methods
        inline void SetGraphicsState(const GraphicsState& state) { m_Impl->SetGraphicsState(state); }
        inline void SetComputeState(const ComputeState& state) { m_Impl->SetComputeState(state); }

        inline void Dispatch(uint32_t groupsX, uint32_t groupsY = 1, uint32_t groupsZ = 1) const { m_Impl->Dispatch(groupsX, groupsY, groupsZ); }

        inline void SetViewport(const Viewport& viewport) const { m_Impl->SetViewport(viewport); }
        inline void SetScissor(const ScissorRect& scissor) const { m_Impl->SetScissor(scissor); }

        inline void BindVertexBuffer(const Buffer& buffer) const { m_Impl->BindVertexBuffer(buffer); }
        inline void BindIndexBuffer(const Buffer& buffer) const { m_Impl->BindIndexBuffer(buffer); }

        inline void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, Image& src, const ImageSliceSpecification& srcSlice) { m_Impl->CopyImage(dst, dstSlice, src, srcSlice); }
        inline void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, StagingImage& src, const ImageSliceSpecification& srcSlice) { m_Impl->CopyImage(dst, dstSlice, src, srcSlice); }
        inline void CopyBuffer(Buffer& dst, Buffer& src, size_t size, size_t srcOffset = 0, size_t dstOffset = 0) { m_Impl->CopyBuffer(dst, src, size, srcOffset, dstOffset); }

        // Draw methods
        inline void DrawIndexed(const DrawArguments& args) const { m_Impl->DrawIndexed(args); }

        // Getters
        inline const CommandListSpecification& GetSpecification() const { return m_Impl->GetSpecification(); }

    public: //private:
        // Constructor
        inline CommandList(CommandListPool& pool, const CommandListSpecification& specs) { m_Impl.Construct(pool, specs); }

    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class CommandListPool;
        friend class APICaster;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // CommandListPool
    ////////////////////////////////////////////////////////////////////////////////////
    class CommandListPool
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanCommandListPool>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12CommandListPool>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyCommandListPool>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyCommandListPool>
        >;
    public:
        // Destructor
        ~CommandListPool() = default;

        // Creation methods // Note: Copy elision (RVO/NRVO) ensures object is constructed directly in the caller's stack frame.
        inline CommandList AllocateList(const CommandListSpecification& specs) { return CommandList(*this, specs); }
        inline void FreeList(CommandList& list) const { m_Impl->FreeList(list); }
        inline void FreeLists(std::span<CommandList*> lists) const { m_Impl->FreeLists(lists); }

        // Helper methods
        inline void Reset() const { m_Impl->Reset(); }

        // Getters
        inline const CommandListPoolSpecification& GetSpecification() const { return m_Impl->GetSpecification(); }

    public: //private:
        // Constructor
        inline CommandListPool(Swapchain& swapchain, const CommandListPoolSpecification& specs) { m_Impl.Construct(swapchain, specs); }

    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class Swapchain;
        friend class APICaster;
    };

}