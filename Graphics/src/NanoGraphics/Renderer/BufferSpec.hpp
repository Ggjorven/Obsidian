#pragma once

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include <cstdint>
#include <string_view>

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // VertexAttributeSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct VertexAttributeSpecification
    {
    public:
        inline constexpr static uint32_t AutoSize = std::numeric_limits<uint32_t>::max();
        inline constexpr static uint32_t AutoOffset = std::numeric_limits<uint32_t>::max();
    public:
        uint32_t BufferIndex = 0;
        Format VertexFormat = Format::Unknown;

        uint32_t Size = 0;
        uint32_t Offset = 0;
        uint32_t ArraySize = 1;

        bool IsInstanced = false;

        std::string_view DebugName = {};

    public:
        // Setters
        inline constexpr VertexAttributeSpecification& SetBufferIndex(uint32_t index) { BufferIndex = index; return *this; }
        inline constexpr VertexAttributeSpecification& SetFormat(Format format) { VertexFormat = format; return *this; }
        
        inline constexpr VertexAttributeSpecification& SetSize(uint32_t size) { Size = size; return *this; }
        inline constexpr VertexAttributeSpecification& SetOffset(uint32_t offset) { Offset = offset; return *this; }
        inline constexpr VertexAttributeSpecification& SetArraySize(uint32_t size) { ArraySize = size; return *this; }

        inline constexpr VertexAttributeSpecification& SetIsInstanced(bool enabled) { IsInstanced = enabled; return *this; }

        inline constexpr VertexAttributeSpecification& SetDebugName(std::string_view name) { DebugName = name; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // BufferSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    class BufferSpecification
    {
    public:
        size_t Size = 0;
        //Format BufferFormat = Format::Unknown;

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
        //inline constexpr BufferSpecification& SetFormat(Format format) { BufferFormat = format; return *this; }

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