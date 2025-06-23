#pragma once

#include <cstdlib>
#include <new>
#include <string>

#include <tracy/Tracy.hpp>

namespace Nano::Graphics::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// Macros
	////////////////////////////////////////////////////////////////////////////////////
	// Settings
	#define NG_ENABLE_PROFILING 0
	#define NG_MEM_PROFILING 0

	// Profiling macros
	#if !defined(NG_CONFIG_DIST) && NG_ENABLE_PROFILING
		#define NG_MARK_FRAME() FrameMark

		#define NG_PROFILE(name) ZoneScopedN(name)

		#if NG_MEM_PROFILING
			void* operator new(size_t size);
			void operator delete(void* ptr) noexcept;
			void operator delete(void* ptr, size_t size) noexcept;
		#endif
	#else
		#define NG_MARK_FRAME()

		#define NG_PROFILE(name)
	#endif

}