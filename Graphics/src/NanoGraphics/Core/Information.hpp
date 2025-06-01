#pragma once

#include <Nano/Nano.hpp>

namespace Nano::Graphics::Information
{

    namespace Structs
    {

        ////////////////////////////////////////////////////////////////////////////////////
        // Structs
        ////////////////////////////////////////////////////////////////////////////////////
        enum class Configuration : uint8_t
        {
            Debug = 0,
            Release,
            Dist
        };

        enum class RenderingAPI : uint8_t
        {
            OpenGL = 0,
            Vulkan,
            D3D11, D3D12
        };

    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Values
    ////////////////////////////////////////////////////////////////////////////////////
    #if defined(NG_CONFIG_DEBUG)
        inline constexpr const Structs::Configuration Configuration = Structs::Configuration::Debug;
    #elif defined(NG_CONFIG_RELEASE)
        inline constexpr const Structs::Configuration Configuration = Structs::Configuration::Release;
    #elif defined(NG_CONFIG_DIST)
        inline constexpr const Structs::Configuration Configuration = Structs::Configuration::Dist;
    #else
        #error Nano Graphics Settings: Unsupported configuration/Failed to query...
    #endif

    // Rendering API choice
    inline constexpr const Structs::RenderingAPI RenderingAPI = Structs::RenderingAPI::Vulkan;

}