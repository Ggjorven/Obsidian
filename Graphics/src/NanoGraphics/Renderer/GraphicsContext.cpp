#include "ngpch.h"
#include "GraphicsContext.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Getters
    ////////////////////////////////////////////////////////////////////////////////////
    bool GraphicsContext::Initialized()
    {
        return s_Initialized;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Static methods
    ////////////////////////////////////////////////////////////////////////////////////
    void GraphicsContext::Init(void* window)
    {
        NG_ASSERT(window, "[GraphicsContext] No valid window was passed in.");

        s_GraphicsContext.Init(window);
        s_Initialized = true;
    }

    void GraphicsContext::Destroy()
    {
        s_GraphicsContext.Destroy();
        s_Initialized = false;
    }

}