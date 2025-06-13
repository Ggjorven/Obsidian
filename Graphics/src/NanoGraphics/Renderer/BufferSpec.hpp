#pragma once

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include <cstdint>
#include <string_view>

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // BufferSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    class BufferSpecification
    {
    public:
        size_t Size = 0;
        Format BufferFormat = Format::Unknown;

        bool IsVertexBuffer = false;
        bool IsIndexBuffer = false;
        bool IsUniformBuffer = false;
        //bool IsAccelStructBuildInput = false;
        //bool IsAccelStructStorage = false;

        bool IsUnorderedAccessed = false;

        ResourceState State = ResourceState::Unknown;
        bool KeepResourceState = false; // Note: After executing commands will go back to set ResourceState ^

        CpuAccessMode CpuAccess = CpuAccessMode::None;

        std::string_view DebugName = {};

    public:
        // Setters
        inline constexpr BufferSpecification& SetSize(size_t size) { Size = size; return *this; }
        inline constexpr BufferSpecification& SetFormat(Format format) { BufferFormat = format; return *this; }

        inline constexpr BufferSpecification& SetIsVertexBuffer(bool enabled) { IsVertexBuffer = enabled; return *this; }
        inline constexpr BufferSpecification& SetIsIndexBuffer(bool enabled) { IsIndexBuffer = enabled; return *this; }
        inline constexpr BufferSpecification& SetIsUniformBuffer(bool enabled) { IsUniformBuffer = enabled; return *this; }

        inline constexpr BufferSpecification& SetIsUnorderedAccessed(bool enabled) { IsUnorderedAccessed = enabled; return *this; }
        inline constexpr BufferSpecification& SetResourceState(ResourceState state) { State = state; return *this; }
        inline constexpr BufferSpecification& SetKeepResourceState(bool enabled) { KeepResourceState = enabled; return *this; }
        inline constexpr BufferSpecification& SetCPUAccess(CpuAccessMode access) { CpuAccess = access; return *this; }
        inline constexpr BufferSpecification& SetDebugName(std::string_view name) { DebugName = name; return *this; }
    };

}