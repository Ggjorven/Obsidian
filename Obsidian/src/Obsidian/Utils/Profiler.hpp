#pragma once

#include <cstdlib>
#include <new>
#include <string>

#include <tracy/Tracy.hpp>

namespace Obsidian::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// Macros
	////////////////////////////////////////////////////////////////////////////////////
	// Settings
	#define OB_ENABLE_PROFILING 0
	#define OB_MEM_PROFILING 0

	// Profiling macros
	#if !defined(OB_CONFIG_DIST) && OB_ENABLE_PROFILING
		#define OB_MARK_FRAME() FrameMark

		#define OB_PROFILE(name) ZoneScopedN(name)

		#if OB_MEM_PROFILING
			void* operator new(size_t size);
			void operator delete(void* ptr) noexcept;
			void operator delete(void* ptr, size_t size) noexcept;
		#endif
	#else
		#define OB_MARK_FRAME()

		#define OB_PROFILE(name)
	#endif

}