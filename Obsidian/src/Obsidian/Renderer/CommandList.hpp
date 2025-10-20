#pragma once
#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/API.hpp"
#include "Obsidian/Renderer/ImageSpec.hpp"
#include "Obsidian/Renderer/CommandListSpec.hpp"

#include "Obsidian/Platform/Vulkan/VulkanCommandList.hpp"
#include "Obsidian/Platform/Dx12/Dx12CommandList.hpp"
#include "Obsidian/Platform/Dummy/DummyCommandList.hpp"

#include <Nano/Nano.hpp>

#include <span>

namespace Obsidian
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
        using Type = Nano::Types::SelectorType<Information::RenderingAPI,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanCommandList>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12CommandList>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyCommandList>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyCommandList>
        >;
    public:
        // Destructor
        ~CommandList() = default;

        // Methods
        inline void Open() { m_Impl->Open(); }
        inline void Close() { m_Impl->Close(); }

        inline void Submit(const CommandListSubmitArgs& args = CommandListSubmitArgs()) { return m_Impl->Submit(args); }

        inline void WaitTillComplete() const { m_Impl->WaitTillComplete(); }

        inline void CommitBarriers() { m_Impl->CommitBarriers(); }

        // Object methods
        inline void StartRenderpass(const RenderpassStartArgs& args) { m_Impl->StartRenderpass(args); }
        inline void EndRenderpass(const RenderpassEndArgs& args) { m_Impl->EndRenderpass(args); }

        inline void BindPipeline(const GraphicsPipeline& pipeline) { m_Impl->BindPipeline(pipeline); }
        inline void BindPipeline(const ComputePipeline& pipeline) { m_Impl->BindPipeline(pipeline); }

        inline void BindBindingSet(const BindingSet& set, std::span<const uint32_t> dynamicOffsets = {}) { m_Impl->BindBindingSet(set, dynamicOffsets); }
        inline void BindBindingSets(std::span<const BindingSet*> sets, std::span<const std::span<const uint32_t>> dynamicOffsets = {}) { m_Impl->BindBindingSets(sets, dynamicOffsets); }

        inline void SetViewport(const Viewport& viewport) const { m_Impl->SetViewport(viewport); }
        inline void SetScissor(const ScissorRect& scissor) const { m_Impl->SetScissor(scissor); }

        inline void BindVertexBuffer(const Buffer& buffer) const { m_Impl->BindVertexBuffer(buffer); }
        inline void BindIndexBuffer(const Buffer& buffer) const { m_Impl->BindIndexBuffer(buffer); }

        inline void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, Image& src, const ImageSliceSpecification& srcSlice) { m_Impl->CopyImage(dst, dstSlice, src, srcSlice); }
        inline void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, StagingImage& src, const ImageSliceSpecification& srcSlice) { m_Impl->CopyImage(dst, dstSlice, src, srcSlice); }
        inline void CopyBuffer(Buffer& dst, Buffer& src, size_t size, size_t srcOffset = 0, size_t dstOffset = 0) { m_Impl->CopyBuffer(dst, src, size, srcOffset, dstOffset); }

        inline void Dispatch(uint32_t groupsX, uint32_t groupsY = 1, uint32_t groupsZ = 1) const { m_Impl->Dispatch(groupsX, groupsY, groupsZ); }

        // State methods // Note: These methods should only be used in very special cases,
        // because internal methods change the state all the time based on needs. Make sure you know what you are doing.
        inline void RequireState(Image& image, const ImageSubresourceSpecification& subresources, ResourceState state) { m_Impl->RequireState(image, subresources, state); }
        inline void RequireState(Buffer& buffer, ResourceState state) { m_Impl->RequireState(buffer, state); }

        // Draw methods
        inline void DrawIndexed(const DrawArguments& args) const { m_Impl->DrawIndexed(args); }

        // Other methods
        inline void PushConstants(const void* memory, size_t size, size_t srcOffset = 0, size_t dstOffset = 0) { m_Impl->PushConstants(memory, size, srcOffset, dstOffset); }

        // Getters
        inline const CommandListSpecification& GetSpecification() const { return m_Impl->GetSpecification(); }

    public: //private:
        // Constructor
        inline CommandList(CommandListPool& pool, const CommandListSpecification& specs = CommandListSpecification()) { m_Impl.Construct(pool, specs); }

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
        using Type = Nano::Types::SelectorType<Information::RenderingAPI,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanCommandListPool>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12CommandListPool>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyCommandListPool>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyCommandListPool>
        >;
    public:
        // Destructor
        ~CommandListPool() = default;

        // Creation methods // Note: Copy elision (RVO/NRVO) ensures object is constructed directly in the caller's stack frame.
        inline CommandList AllocateList(const CommandListSpecification& specs = CommandListSpecification()) { return CommandList(*this, specs); }
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