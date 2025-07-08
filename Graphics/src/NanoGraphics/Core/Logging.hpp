#pragma once

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{

	////////////////////////////////////////////////////////////////////////////////////
	// Logging & Asserting macros
	////////////////////////////////////////////////////////////////////////////////////
    #ifndef NG_CONFIG_DIST
        #define NG_LOG_TRACE(fmt, ...)       ::Nano::Log::PrintLvl<::Nano::Log::Level::Trace>(fmt __VA_OPT__(,) __VA_ARGS__)
        #define NG_LOG_INFO(fmt, ...)        ::Nano::Log::PrintLvl<::Nano::Log::Level::Info>(fmt __VA_OPT__(,) __VA_ARGS__)
        #define NG_LOG_WARN(fmt, ...)        ::Nano::Log::PrintLvl<::Nano::Log::Level::Warn>(fmt __VA_OPT__(,) __VA_ARGS__)
        #define NG_LOG_ERROR(fmt, ...)       ::Nano::Log::PrintLvl<::Nano::Log::Level::Error>(fmt __VA_OPT__(,) __VA_ARGS__)
        #define NG_LOG_FATAL(fmt, ...)       ::Nano::Log::PrintLvl<::Nano::Log::Level::Fatal>(fmt __VA_OPT__(,) __VA_ARGS__)

        #define NG_ASSERT(x, fmt, ...)          \
                    do                          \
                    {                           \
                        if (!(x))               \
                        {                       \
                            NG_LOG_FATAL("Assertion failed: ({0}), {1}.", #x, ::Nano::Text::Format(fmt __VA_OPT__(,) __VA_ARGS__)); \
                            NANO_DEBUG_BREAK(); \
                        }                       \
                    } while (false)

        #define NG_VERIFY(x, fmt, ...)          \
                    do                          \
                    {                           \
                        if (!(x))               \
                        {                       \
                            NG_LOG_FATAL("Verify failed: ({0}), {1}.", #x, ::Nano::Text::Format(fmt __VA_OPT__(,) __VA_ARGS__)); \
                        }                       \
                    } while (false)

        #define NG_UNREACHABLE() NG_ASSERT(false, "Unreachable")

    #else
        #define NG_LOG_TRACE(fmt, ...) 
        #define NG_LOG_INFO(fmt, ...) 
        #define NG_LOG_WARN(fmt, ...) 
        #define NG_LOG_ERROR(fmt, ...) 
        #define NG_LOG_FATAL(fmt, ...) 

        #define NG_ASSERT(x, fmt, ...)
        #define NG_VERIFY(x, fmt, ...)
        #define NG_UNREACHABLE()
    #endif

}