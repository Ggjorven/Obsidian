#pragma once

#include <Nano/Nano.hpp>

#include <cstdint>

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Flags
    ////////////////////////////////////////////////////////////////////////////////////
    enum class ResourceState : uint16_t //uint32_t
    {
        Unknown = 0,

        Common = 1 << 0,
        StorageBuffer = 1 << 1,
        VertexBuffer = 1 << 2,
        IndexBuffer = 1 << 3,
        IndirectArgument = 1 << 4,
        ShaderResource = 1 << 5,
        UnorderedAccess = 1 << 6,
        RenderTarget = 1 << 7,
        DepthWrite = 1 << 8,
        DepthRead = 1 << 9,
        StreamOut = 1 << 10,
        CopyDst = 1 << 11,
        CopySrc = 1 << 12,
        Present = 1 << 13,
        //AccelStructRead = 1 << 14,
        //AccelStructWrite = 1 << 15,
        //AccelStructBuildInput = 1 << 16,
        //AccelStructBuildBlas = 1 << 17,
    };

    NANO_DEFINE_BITWISE(ResourceState)

    enum class CpuAccessMode : uint8_t
    {
        None = 0,
        Read = 1 << 0,
        Write = 1 << 1
    };

    NANO_DEFINE_BITWISE(CpuAccessMode)

}