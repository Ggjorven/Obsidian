#pragma once

#include "NanoGraphics/Core/Logging.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"
#include "NanoGraphics/Renderer/RenderpassSpec.hpp"

#include <Nano/Nano.hpp>

#include <cstdint>
#include <span>
#include <cmath>
#include <array>
#include <vector>
#include <variant>
#include <string_view>
#include <initializer_list>

namespace Nano::Graphics
{

    class BindingSet;
    class Image;
    class Buffer;
    class Renderpass;
    class Framebuffer;
    class GraphicsPipeline;
    class ComputePipeline;
    class CommandList;

    ////////////////////////////////////////////////////////////////////////////////////
    // Flags
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
        inline constexpr CommandListSubmitArgs& SetWaitOnLists(std::span<const CommandList*> viewedLists) { WaitOnLists = viewedLists; return *this; }
        inline constexpr CommandListSubmitArgs& SetWaitForSwapchainImage(bool enabled) { WaitForSwapchainImage = enabled; return *this; }
        inline constexpr CommandListSubmitArgs& SetOnFinishMakeSwapchainPresentable(bool enabled) { OnFinishMakeSwapchainPresentable = enabled; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // DrawArguments
    ////////////////////////////////////////////////////////////////////////////////////
    struct DrawArguments
    {
    public:
        uint32_t VertexCount = 0;
        uint32_t InstanceCount = 1;
        uint32_t StartIndexLocation = 0;
        uint32_t StartVertexLocation = 0;
        uint32_t StartInstanceLocation = 0;

    public:
        inline constexpr DrawArguments& SetVertexCount(uint32_t count) { VertexCount = count; return *this; }
        inline constexpr DrawArguments& SetInstanceCount(uint32_t count) { InstanceCount = count; return *this; }
        inline constexpr DrawArguments& SetStartIndexLocation(uint32_t location) { StartIndexLocation = location; return *this; }
        inline constexpr DrawArguments& SetStartVertexLocation(uint32_t location) { StartVertexLocation = location; return *this; }
        inline constexpr DrawArguments& SetStartInstanceLocation(uint32_t location) { StartInstanceLocation = location; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // GraphicsState
    ////////////////////////////////////////////////////////////////////////////////////
    struct GraphicsState
    {
    public:
        inline constexpr static uint32_t MaxBindingSets = 8;
    public:
        struct BindPair
        {
        public:
            BindingSet* Set = nullptr;
            std::span<const uint32_t> DynamicOffsets = {}; // Note: For Dynamic UniformBuffer for example (use the buffer's m_Specification.Size per element)
        };
    public:
        GraphicsPipeline* Pipeline = nullptr;
        Renderpass* Pass = nullptr;
        Framebuffer* Frame = nullptr; // Note: Can be nullptr, will get Framebuffer[CurrentFrame] from pass.

        Viewport ViewportState = {};
        ScissorRect Scissor = {};

        Maths::Vec4<float> ColourClear = { 0.0f, 0.0f, 0.0f, 1.0f };
        float DepthClear = 1.0f;

        std::array<BindPair, MaxBindingSets> BindingSets = { };


    public:
        // Setters
        inline constexpr GraphicsState& SetPipeline(GraphicsPipeline& pipeline) { Pipeline = &pipeline; return *this; }
        inline constexpr GraphicsState& SetRenderpass(Renderpass& renderpass) { Pass = &renderpass; return *this; }
        inline constexpr GraphicsState& SetFramebuffer(Framebuffer& framebuffer) { Frame = &framebuffer; return *this; }
    
        inline constexpr GraphicsState& SetViewport(const Viewport& viewport) { ViewportState = viewport; return *this; }
        inline constexpr GraphicsState& SetScissor(const ScissorRect& scissor) { Scissor = scissor; return *this; }

        inline constexpr GraphicsState& SetColourClear(const Maths::Vec4<float>& colour) { ColourClear = colour; return *this; }
        inline constexpr GraphicsState& SetDepthClear(float depth) { DepthClear = depth; return *this; }

        inline GraphicsState& AddBindingSet(uint32_t setID, BindingSet& set, std::span<const uint32_t> dynamicOffsets = {}) { NG_ASSERT((setID < MaxBindingSets), "[GraphicsState] SetID exceeds max binding sets."); BindingSets[setID] = { &set, dynamicOffsets }; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ComputeState
    ////////////////////////////////////////////////////////////////////////////////////
    struct ComputeState
    {
    public:
        inline constexpr static uint32_t MaxBindingSets = GraphicsState::MaxBindingSets;
        using BindPair = GraphicsState::BindPair;
    public:
        ComputePipeline* Pipeline = nullptr;

        std::array<BindPair, MaxBindingSets> BindingSets = { };

    public:
        // Setters
        inline constexpr ComputeState& SetPipeline(ComputePipeline& pipeline) { Pipeline = &pipeline; return *this; }

        inline ComputeState& AddBindingSet(uint32_t setID, BindingSet& set, std::span<const uint32_t> dynamicOffsets = {}) { NG_ASSERT((setID < MaxBindingSets), "[ComputeState] SetID exceeds max binding sets."); BindingSets[setID] = { &set, dynamicOffsets }; return *this; }
    };

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

}