#pragma once

#include <Nano/Nano.hpp>

#include <cstdint>

namespace Obsidian
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Flags
    ////////////////////////////////////////////////////////////////////////////////////
    enum class ResourceState : uint16_t
    {
        Unknown = 0,
        Common = Unknown,

        StorageBuffer = 1 << 0,
        VertexBuffer = 1 << 1,
        IndexBuffer = 1 << 2,
        IndirectArgument = 1 << 3,
        ShaderResource = 1 << 4,
        SRV = ShaderResource,
        UnorderedAccess = 1 << 5,
        UAV = UnorderedAccess,
        ColourAttachment = 1 << 6,
        RenderTarget = ColourAttachment,
        DepthWrite = 1 << 7,
        DepthRead = 1 << 8,
        CopyDst = 1 << 9,
        CopySrc = 1 << 10,
        Present = 1 << 11,
        //AccelStructRead = 1 << 12,
        //AccelStructWrite = 1 << 13,
        //AccelStructBuildInput = 1 << 14,
        //AccelStructBuildBlas = 1 << 15,
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
        if (static_cast<bool>(state & ResourceState::CopyDst))
            str += (str.empty() ? "" : " | ") + std::string("CopyDst");
        if (static_cast<bool>(state & ResourceState::CopySrc))
            str += (str.empty() ? "" : " | ") + std::string("CopySrc");
        if (static_cast<bool>(state & ResourceState::Present))
            str += (str.empty() ? "" : " | ") + std::string("Present");

        return (str.empty() ? "Unknown" : str);
    }

}