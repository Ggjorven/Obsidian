#pragma once

#include "Obsidian/Renderer/ResourceSpec.hpp"
#include "Obsidian/Renderer/ImageSpec.hpp"

#include <cstdint>
#include <numeric>
#include <limits>
#include <string>

namespace Obsidian
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
        inline constexpr static size_t DefaultVertexBufferAlignment = 16ull;
        inline constexpr static size_t DefaultIndexBufferAlignment = 4ull;
        inline constexpr static size_t DefaultUniformBufferAlignment = 256ull;
        inline constexpr static size_t DefaultStorageBufferAlignment = DefaultUniformBufferAlignment;
        inline constexpr static size_t DefaultTexelBufferAlignment = 16ull;
    public:
        size_t Size = 0; // Note: Size in bytes, must not be specified for Dynamic/Volatile buffers. Will be ignored.
        
        size_t Stride = 0; // Note: Size of a struct, needed for StorageBuffer/StructuredBuffer and Dynamic/Volatile buffers
        uint32_t ElementCount = 0; // Note: Max amount of elements for Dynamic/Volatile buffers, not needed for other buffers.

        Format BufferFormat = Format::Unknown; // Note: Only necessary for Indexbuffer & Texelbuffer, like R16UInt or R32UInt

        bool IsVertexBuffer : 1 = false;
        bool IsIndexBuffer : 1 = false;
        bool IsUniformBuffer : 1 = false;
        //bool IsAccelStructBuildInput = false;
        //bool IsAccelStructStorage = false;

        bool IsDynamic : 1 = false; // For Dx12 IsVolatile
        bool IsTexel : 1 = false; // For Dx12 IsTyped
        bool IsUnorderedAccessed : 3 = false;

        ResourceState PermanentState = ResourceState::Unknown; // Note: Anything other than Unknown sets it to be permanent

        CpuAccessMode CpuAccess = CpuAccessMode::None;

        std::string DebugName = {};

    public:
        // Setters
        inline constexpr BufferSpecification& SetSize(size_t size) { Size = size; return *this; }

        inline constexpr BufferSpecification& SetStride(size_t stride) { Stride = stride; return *this; }
        inline constexpr BufferSpecification& SetElementCount(uint32_t count) { ElementCount = count; return *this; }

        inline constexpr BufferSpecification& SetFormat(Format format) { BufferFormat = format; return *this; }

        inline constexpr BufferSpecification& SetIsVertexBuffer(bool enabled) { IsVertexBuffer = enabled; return *this; }
        inline constexpr BufferSpecification& SetIsIndexBuffer(bool enabled) { IsIndexBuffer = enabled; return *this; }
        inline constexpr BufferSpecification& SetIsUniformBuffer(bool enabled) { IsUniformBuffer = enabled; return *this; }
        inline constexpr BufferSpecification& SetIsContantBuffer(bool enabled) { IsUniformBuffer = enabled; return *this; }

        inline constexpr BufferSpecification& SetIsDynamic(bool enabled) { IsDynamic = enabled; return *this; }
        inline constexpr BufferSpecification& SetIsVolatile(bool enabled) { IsDynamic = enabled; return *this; }
        inline constexpr BufferSpecification& SetIsTexel(bool enabled) { IsTexel = enabled; return *this; }
        inline constexpr BufferSpecification& SetIsTyped(bool enabled) { IsTexel = enabled; return *this; }
        inline constexpr BufferSpecification& SetIsUnorderedAccessed(bool enabled) { IsUnorderedAccessed = enabled; return *this; }
        inline constexpr BufferSpecification& SetIsUAV(bool enabled) { IsUnorderedAccessed = enabled; return *this; }

        inline constexpr BufferSpecification& SetPermanentState(ResourceState state) { PermanentState = state; return *this; }
        inline constexpr BufferSpecification& SetCPUAccess(CpuAccessMode access) { CpuAccess = access; return *this; }
        inline BufferSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }

        inline constexpr bool HasPermanentState() const { return (PermanentState != ResourceState::Unknown); }
    };

}