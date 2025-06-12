#pragma once
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"
#include "NanoGraphics/Renderer/CommandListSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanCommandList.hpp"

#include <Nano/Nano.hpp>

#include <span>

namespace Nano::Graphics
{

    class Swapchain;
    class Image;
    class CommandListPool;

    ////////////////////////////////////////////////////////////////////////////////////
    // CommandList
    ////////////////////////////////////////////////////////////////////////////////////
    class CommandList
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanCommandList>
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

        // Object methods
        inline void SetGraphicsState(const GraphicsState& state) { m_CommandList.SetGraphicsState(state); }

        inline void CopyImage(Image& dst, const ImageSliceSpecification& dstSlice, Image& src, const ImageSliceSpecification& srcSlice) { m_CommandList.CopyImage(dst, dstSlice, src, srcSlice); }

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
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanCommandListPool>
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