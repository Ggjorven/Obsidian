#pragma once

#include <Nano/Nano.hpp>

namespace Obsidian::Information
{

    namespace Structs
    {

        ////////////////////////////////////////////////////////////////////////////////////
        // Structs
        ////////////////////////////////////////////////////////////////////////////////////
        enum class Configuration : uint8_t
        {
            Unknown = 0,

            Debug,
            Release,
            Dist
        };

        enum class Platform : uint8_t
        {
            Windows = 0,
            Linux,
            MacOS,

            Android,
            iOS,
        };

        enum class RenderingAPI : uint8_t
        {
            Vulkan = 0,
            Dx12,
            Metal,
            Dummy
        };

    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Redefine macros
    ////////////////////////////////////////////////////////////////////////////////////
    #if defined(NANO_COMPILER_GCC)
        #define OB_COMPILER_GCC
    #elif defined (NANO_COMPILER_CLANG)
        #define OB_COMPILER_CLANG
    #elif defined(NANO_COMPILER_MSVC)
        #define OB_COMPILER_MSVC
    #endif

    ////////////////////////////////////////////////////////////////////////////////////
    // Values
    ////////////////////////////////////////////////////////////////////////////////////
    #if defined(OB_CONFIG_DEBUG)
        inline constexpr const Structs::Configuration Configuration = Structs::Configuration::Debug;
    #elif defined(OB_CONFIG_RELEASE)
        inline constexpr const Structs::Configuration Configuration = Structs::Configuration::Release;
    #elif defined(OB_CONFIG_DIST)
        inline constexpr const Structs::Configuration Configuration = Structs::Configuration::Dist;
    #else
        // Note: This is for submodules/dependencies since I don't want to force
        // them to define preprocessor defines.
        #if defined(NANO_CONFIG_DEBUG)
            inline constexpr const Structs::Configuration Configuration = Structs::Configuration::Debug;
        #elif defined(NANO_CONFIG_RELEASE)
            inline constexpr const Structs::Configuration Configuration = Structs::Configuration::Release;
        #else
            inline constexpr const Structs::Configuration Configuration = Structs::Configuration::Unknown;
        #endif
    #endif

    #if defined(NANO_PLATFORM_WINDOWS)
        #define OB_PLATFORM_WINDOWS
        inline constexpr const Structs::Platform Platform = Structs::Platform::Windows;
    #elif defined(NANO_PLATFORM_LINUX)
        #define OB_PLATFORM_LINUX
        inline constexpr const Structs::Platform Platform = Structs::Platform::Linux;
    #elif defined(NANO_PLATFORM_MACOS)
        #define OB_PLATFORM_MACOS
        inline constexpr const Structs::Platform Platform = Structs::Platform::MacOS;
    #else
        #error Obsidian Settings: Must compile for a desktop platform.
    #endif

    #if defined(NANO_PLATFORM_DESKTOP)
        #define OB_PLATFORM_DESKTOP
    #else
        #error Obsidian Settings: Must compile for a desktop platform.
    #endif
    #if defined(NANO_PLATFORM_POSIX)
        #define OB_PLATFORM_POSIX
    #endif
    #if defined(NANO_PLATFORM_APPLE)
        #define OB_PLATFORM_APPLE
    #endif

    #if defined(OB_API_VULKAN)
        inline constexpr const Structs::RenderingAPI RenderingAPI = Structs::RenderingAPI::Vulkan;
    #elif defined(OB_API_DX12)
        inline constexpr const Structs::RenderingAPI RenderingAPI = Structs::RenderingAPI::Dx12;
    #elif defined(OB_API_METAL)
        inline constexpr const Structs::RenderingAPI RenderingAPI = Structs::RenderingAPI::Metal;
        #error Nano Graphics Settings: Metal not implemented yet.
    #elif defined(OB_API_DUMMY)
        inline constexpr const Structs::RenderingAPI RenderingAPI = Structs::RenderingAPI::Dummy;
    #else
        #error Obsidian Settings: Unsupported API/Failed to query... Must define NG_API_XXX
    #endif

    // Note: This is only used in .cpp files, so as long as the Obsidian library is compiled with the appropriate
    // preprocessor flags it is fine. It also means that when using Obsidian as a submodule this flag
    // won't represent the proper value for a custom Distribution configuration since we can only query debug or no debug without preprocessor directives.
    inline constexpr const bool Validation = (Information::Configuration != Information::Structs::Configuration::Dist);
    
    // Frames In Flight
    inline constexpr const uint8_t FramesInFlight = 3;
    inline constexpr const uint8_t MaxImageCount = 6;

    static_assert((MaxImageCount >= FramesInFlight), "FramesInFlight must be less or equal to the upper limit.");

}