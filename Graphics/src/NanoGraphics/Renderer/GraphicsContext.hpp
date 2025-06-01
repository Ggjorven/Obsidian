#pragma once

#include "NanoGraphics/Platform/Vulkan/VulkanContext.hpp"

#include <Nano/Nano.hpp>

#include <cstdint>

namespace Nano::Graphics
{

	////////////////////////////////////////////////////////////////////////////////////
	// GraphicsContext
	////////////////////////////////////////////////////////////////////////////////////
	// Note: You only need one context for an entire application, even with multiple windows
	// So, only initialize it once. // Note: Currently we only support a single window.
	class GraphicsContext
	{
	public:
		using Type = Internal::VulkanContext;
	public:
		// Getters
		static bool Initialized();

		// Static methods
		static void Init(void* window);
		static void Destroy();

		// Internal
		inline static Type& GetInternalContext() { return s_GraphicsContext; }

	private:
		inline static bool s_Initialized = false;
		inline static Type s_GraphicsContext = {};
	};

}
