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
            Vulkan = 0,
            D3D12,
            Metal,
            Dummy
        };

    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Redefine macros
    ////////////////////////////////////////////////////////////////////////////////////
    #if defined(NANO_COMPILER_GCC)
        #define NG_COMPILER_GCC
    #elif defined (NANO_COMPILER_CLANG)
        #define NG_COMPILER_CLANG
    #elif defined(NANO_COMPILER_MSVC)
        #define NG_COMPILER_MSVC
    #endif

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

    #if defined(NG_API_VULKAN)
        inline constexpr const Structs::RenderingAPI RenderingAPI = Structs::RenderingAPI::Vulkan;
    #elif defined(NG_API_D3D12)
        inline constexpr const Structs::RenderingAPI RenderingAPI = Structs::RenderingAPI::D3D12;
        #error Nano Graphics Settings: D3D12 not implemented yet.
    #elif defined(NG_API_METAL)
        inline constexpr const Structs::RenderingAPI RenderingAPI = Structs::RenderingAPI::Metal;
        #error Nano Graphics Settings: Metal not implemented yet.
    #elif defined(NG_API_DUMMY)
        inline constexpr const Structs::RenderingAPI RenderingAPI = Structs::RenderingAPI::Dummy;
    #else
        #error Nano Graphics Settings: Unsupported API/Failed to query...
    #endif
    
    // Frames In Flight
    inline constexpr const uint8_t BackBufferCount = 3;

}