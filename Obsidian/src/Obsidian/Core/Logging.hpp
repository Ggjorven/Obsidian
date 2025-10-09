#pragma once

#include <Nano/Nano.hpp>

namespace Obsidian
{

	////////////////////////////////////////////////////////////////////////////////////
	// Logging & Asserting macros
	////////////////////////////////////////////////////////////////////////////////////
    #ifndef OB_CONFIG_DIST
        #define OB_LOG_TRACE(fmt, ...)       ::Nano::Log::PrintLvl<::Nano::Log::Level::Trace>(fmt __VA_OPT__(,) __VA_ARGS__)
        #define OB_LOG_INFO(fmt, ...)        ::Nano::Log::PrintLvl<::Nano::Log::Level::Info>(fmt __VA_OPT__(,) __VA_ARGS__)
        #define OB_LOG_WARN(fmt, ...)        ::Nano::Log::PrintLvl<::Nano::Log::Level::Warn>(fmt __VA_OPT__(,) __VA_ARGS__)
        #define OB_LOG_ERROR(fmt, ...)       ::Nano::Log::PrintLvl<::Nano::Log::Level::Error>(fmt __VA_OPT__(,) __VA_ARGS__)
        #define OB_LOG_FATAL(fmt, ...)       ::Nano::Log::PrintLvl<::Nano::Log::Level::Fatal>(fmt __VA_OPT__(,) __VA_ARGS__)

        #if !defined(OB_ASSERT)
            #define OB_ASSERT(x, fmt, ...)          \
                        do                          \
                        {                           \
                            if (!(x))               \
                            {                       \
                                OB_LOG_FATAL("Assertion failed: ({0}), {1}.", #x, ::Nano::Text::Format(fmt __VA_OPT__(,) __VA_ARGS__)); \
                                NANO_DEBUG_BREAK(); \
                            }                       \
                        } while (false)
        #endif

        #define OB_VERIFY(x, fmt, ...)          \
                    do                          \
                    {                           \
                        if (!(x))               \
                        {                       \
                            OB_LOG_FATAL("Verify failed: ({0}), {1}.", #x, ::Nano::Text::Format(fmt __VA_OPT__(,) __VA_ARGS__)); \
                        }                       \
                    } while (false)

        #define OB_UNREACHABLE() OB_ASSERT(false, "Unreachable")

    #else
        #define OB_LOG_TRACE(fmt, ...) 
        #define OB_LOG_INFO(fmt, ...) 
        #define OB_LOG_WARN(fmt, ...) 
        #define OB_LOG_ERROR(fmt, ...) 
        #define OB_LOG_FATAL(fmt, ...) 

        #define OB_ASSERT(x, fmt, ...)
        #define OB_VERIFY(x, fmt, ...)
        #define OB_UNREACHABLE()
    #endif

}