#pragma once

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include <cstdint>
#include <numeric>
#include <limits>
#include <string>

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Structs
    ////////////////////////////////////////////////////////////////////////////////////
    struct BufferRange
    {
    public:
        inline constexpr static uint32_t FullSize = std::numeric_limits<uint32_t>::max();
    public:
        size_t Size = FullSize;
        size_t Offset = 0;

    public:
        // Setters
        inline constexpr BufferRange& SetSize(size_t size) { Size = size; return *this; }
        inline constexpr BufferRange& SetOffset(size_t offset) { Offset = offset; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // VertexAttributeSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct VertexAttributeSpecification
    {
    public:
        inline constexpr static uint32_t AutoSize = std::numeric_limits<uint32_t>::max();
        inline constexpr static uint32_t AutoOffset = std::numeric_limits<uint32_t>::max();
    public:
        uint32_t Location = 0;
        uint32_t BufferIndex = 0;
        Format VertexFormat = Format::Unknown;

        uint32_t Size = 0;
        uint32_t Offset = 0;
        uint32_t ArraySize = 1;

        bool IsInstanced = false;

        std::string DebugName = {};

    public:
        // Setters
        inline constexpr VertexAttributeSpecification& SetLocation(uint32_t location) { Location = location; return *this; }
        inline constexpr VertexAttributeSpecification& SetBufferIndex(uint32_t index) { BufferIndex = index; return *this; }
        inline constexpr VertexAttributeSpecification& SetFormat(Format format) { VertexFormat = format; return *this; }
        
        inline constexpr VertexAttributeSpecification& SetSize(uint32_t size) { Size = size; return *this; }
        inline constexpr VertexAttributeSpecification& SetOffset(uint32_t offset) { Offset = offset; return *this; }
        inline constexpr VertexAttributeSpecification& SetArraySize(uint32_t size) { ArraySize = size; return *this; }

        inline constexpr VertexAttributeSpecification& SetIsInstanced(bool enabled) { IsInstanced = enabled; return *this; }

        inline VertexAttributeSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // BufferSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    class BufferSpecification
    {
    public:
        size_t Size = 0;
        Format BufferFormat = Format::Unknown; // Note: Only necessary for Indexbuffer like R32UInt or R16UInt

        bool IsVertexBuffer = false;
        bool IsIndexBuffer = false;
        bool IsUniformBuffer = false;
        //bool IsAccelStructBuildInput = false;
        //bool IsAccelStructStorage = false;

        bool IsTexel = false;
        bool IsUnorderedAccessed = false;

        ResourceState PermanentState = ResourceState::Unknown; // Note: Anything other than Unknown sets it to be permanent

        CpuAccessMode CpuAccess = CpuAccessMode::None;

        std::string DebugName = {};

    public:
        // Setters
        inline constexpr BufferSpecification& SetSize(size_t size) { Size = size; return *this; }
        inline constexpr BufferSpecification& SetFormat(Format format) { BufferFormat = format; return *this; }

        inline constexpr BufferSpecification& SetIsVertexBuffer(bool enabled) { IsVertexBuffer = enabled; return *this; }
        inline constexpr BufferSpecification& SetIsIndexBuffer(bool enabled) { IsIndexBuffer = enabled; return *this; }
        inline constexpr BufferSpecification& SetIsUniformBuffer(bool enabled) { IsUniformBuffer = enabled; return *this; }
        inline constexpr BufferSpecification& SetIsContantBuffer(bool enabled) { IsUniformBuffer = enabled; return *this; }

        inline constexpr BufferSpecification& SetIsTexel(bool enabled) { IsTexel = enabled; return *this; }
        inline constexpr BufferSpecification& SetIsUnorderedAccessed(bool enabled) { IsUnorderedAccessed = enabled; return *this; }
        inline constexpr BufferSpecification& SetIsUAV(bool enabled) { IsUnorderedAccessed = enabled; return *this; }

        inline constexpr BufferSpecification& SetPermanentState(ResourceState state) { PermanentState = state; return *this; }
        inline constexpr BufferSpecification& SetCPUAccess(CpuAccessMode access) { CpuAccess = access; return *this; }
        inline BufferSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }

        inline constexpr bool HasPermanentState() const { return (PermanentState != ResourceState::Unknown); }
    };

}