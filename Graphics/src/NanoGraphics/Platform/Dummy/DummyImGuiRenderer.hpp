#pragma once

#include <Nano/Nano.hpp>

#include <imgui.h>

namespace Nano::Graphics
{
	class Device;
	class Swapchain;
	class Renderpass;
	class CommandList;
}

namespace Nano::Graphics::Internal
{

	class DummyImGuiRenderer;

#if 1 //defined(NG_API_DUMMY)
	////////////////////////////////////////////////////////////////////////////////////
	// DummyImGuiRenderer
	////////////////////////////////////////////////////////////////////////////////////
	class DummyImGuiRenderer 
	{
	public:
		// Init & Destroy
		inline constexpr DummyImGuiRenderer(const Device& device, const Swapchain& swapchain, const Renderpass& renderpass)
		{ (void)device; (void)swapchain; (void)renderpass; }
		constexpr ~DummyImGuiRenderer() = default;

		// Methods
		inline constexpr void Begin() { }
		inline constexpr void End(const CommandList& commandList) { (void)commandList; }
	};
#endif

}