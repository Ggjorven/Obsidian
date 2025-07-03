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

    ////////////////////////////////////////////////////////////////////////////////////
    // Helper methods
    ////////////////////////////////////////////////////////////////////////////////////
    inline std::string ResourceStateToString(ResourceState state)
    {
        std::string str = {};

        if (static_cast<bool>(state & ResourceState::Common))
            str += "Common";
        if (static_cast<bool>(state & ResourceState::StorageBuffer))
            str += (str.empty() ? "" : " | ") + std::string("StorageBuffer");
        if (static_cast<bool>(state & ResourceState::VertexBuffer))
            str += (str.empty() ? "" : " | ") + std::string("VertexBuffer");
        if (static_cast<bool>(state & ResourceState::IndexBuffer))
            str += (str.empty() ? "" : " | ") + std::string("IndexBuffer");
        if (static_cast<bool>(state & ResourceState::IndirectArgument))
            str += (str.empty() ? "" : " | ") + std::string("IndirectArgument");
        if (static_cast<bool>(state & ResourceState::ShaderResource))
            str += (str.empty() ? "" : " | ") + std::string("ShaderResource");
        if (static_cast<bool>(state & ResourceState::UnorderedAccess))
            str += (str.empty() ? "" : " | ") + std::string("UnorderedAccess");
        if (static_cast<bool>(state & ResourceState::RenderTarget))
            str += (str.empty() ? "" : " | ") + std::string("RenderTarget");
        if (static_cast<bool>(state & ResourceState::DepthWrite))
            str += (str.empty() ? "" : " | ") + std::string("DepthWrite");
        if (static_cast<bool>(state & ResourceState::DepthRead))
            str += (str.empty() ? "" : " | ") + std::string("DepthRead");
        if (static_cast<bool>(state & ResourceState::StreamOut))
            str += (str.empty() ? "" : " | ") + std::string("StreamOut");
        if (static_cast<bool>(state & ResourceState::CopyDst))
            str += (str.empty() ? "" : " | ") + std::string("CopyDst");
        if (static_cast<bool>(state & ResourceState::CopySrc))
            str += (str.empty() ? "" : " | ") + std::string("CopySrc");
        if (static_cast<bool>(state & ResourceState::Present))
            str += (str.empty() ? "" : " | ") + std::string("Present");

        return (str.empty() ? "Unknown" : str);
    }

}