#pragma once

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"

#include <Nano/Nano.hpp>

#include <cstdint>
#include <span>
#include <cmath>
#include <vector>
#include <variant>
#include <string_view>
#include <initializer_list>

namespace Nano::Graphics
{

    class CommandList;

    ////////////////////////////////////////////////////////////////////////////////////
    // CommandQueue
    ////////////////////////////////////////////////////////////////////////////////////
    enum class CommandQueue : uint8_t
    {
        Graphics = 0,
        Compute,
        Present,

        Count
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // CommandListSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct CommandListSpecification
    {
    public:
        std::string_view DebugName = {};

    public:
        // Setters
        inline constexpr CommandListSpecification& SetDebugName(std::string_view name) { DebugName = name; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // CommandListPoolSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct CommandListPoolSpecification
    {
    public:
        std::string_view DebugName = {};

    public:
        // Setters
        inline constexpr CommandListPoolSpecification& SetDebugName(std::string_view name) { DebugName = name; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // CommandListSubmitArgs
    ////////////////////////////////////////////////////////////////////////////////////
    struct CommandListSubmitArgs
    {
    public:
        CommandQueue Queue = CommandQueue::Graphics;
        
        std::variant<std::vector<const CommandList*>, std::span<const CommandList*>> WaitOnLists = {};
        
        bool WaitForSwapchainImage = false;
        bool OnFinishMakeSwapchainPresentable = false;

    public:
        // Setters
        inline constexpr CommandListSubmitArgs& SetQueue(CommandQueue queue) { Queue = queue; return *this; }
        inline CommandListSubmitArgs& SetWaitOnLists(std::vector<const CommandList*>&& ownedLists) { WaitOnLists = std::move(ownedLists); return *this; }
        inline CommandListSubmitArgs& SetWaitOnLists(const std::vector<const CommandList*>& ownedLists) { WaitOnLists = ownedLists; return *this; }
        inline CommandListSubmitArgs& SetWaitOnLists(std::initializer_list<const CommandList*> ownedLists) { WaitOnLists = ownedLists; return *this; }
        inline CommandListSubmitArgs& SetWaitOnLists(std::span<const CommandList*> viewedLists) { WaitOnLists = viewedLists; return *this; }
        inline constexpr CommandListSubmitArgs& SetWaitForSwapchainImage(bool enabled) { WaitForSwapchainImage = enabled; return *this; }
        inline constexpr CommandListSubmitArgs& SetOnFinishMakeSwapchainPresentable(bool enabled) { OnFinishMakeSwapchainPresentable = enabled; return *this; }
    };

}