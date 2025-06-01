#include "ngpch.h"
#include "Profiler.hpp"

#include "NanoGraphics/Core/Logging.hpp"

#if !defined(NG_CONFIG_DIST) && NG_ENABLE_PROFILING
    #if NG_MEM_PROFILING
    void* operator new(size_t size)
    {
        auto ptr = std::malloc(size);
        if (!ptr)
            throw std::bad_alloc();

        TracyAlloc(ptr, size);
        return ptr;
    }

    void operator delete(void* ptr) noexcept
    {
        TracyFree(ptr);
        std::free(ptr);
    }

    void operator delete(void* ptr, size_t size) noexcept
    {
        TracyFree(ptr);
        std::free(ptr);
    }
    #endif
#endif