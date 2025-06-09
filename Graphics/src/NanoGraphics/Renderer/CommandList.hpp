#pragma once
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Renderer/CommandListSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanCommandList.hpp"

#include <Nano/Nano.hpp>

#include <span>

namespace Nano::Graphics
{

    class ExecutionRegion;
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
        inline void Begin(bool reset = true) const { return m_CommandList.Begin(reset); }
        inline void End() const { return m_CommandList.End(); }
        inline void Submit(const CommandListSubmitArgs& args) const { return m_CommandList.Submit(args); }

    private:
        // Constructor
        inline CommandList(const CommandListPool& pool, const CommandListSpecification& specs)
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
        inline CommandList AllocateList(const CommandListSpecification& specs) const { return CommandList(*this, specs); }
        inline void FreeList(CommandList& list) const { m_Pool.FreeList(list); }
        inline void FreeLists(std::span<CommandList*> lists) const { m_Pool.FreeLists(lists); }

        // Helper methods
        inline void ResetList(CommandList& list) const { m_Pool.ResetList(list); }
        inline void ResetAll() const { m_Pool.ResetAll(); }

    private:
        // Constructor
        inline CommandListPool(const ExecutionRegion& execRegion, const CommandListPoolSpecification& specs)
            : m_Pool(execRegion, specs) {}

    private:
        Type m_Pool;

        friend class ExecutionRegion;
    };

}