#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Window.hpp"

using namespace Nano::Graphics;

int Main(int argc, char* argv[])
{
	Window window({
		.Title = "First",

		.Width = 1280,
		.Height = 720,

		.EventCallback = [](Event e) {},

		.VSync = false,
	});

	NG_LOG_TRACE("Trace");
	NG_LOG_INFO("Info {0} {1} {2}", 1, 2, 3);
	NG_LOG_WARN("Warn");
	NG_LOG_ERROR("Error");
	NG_LOG_FATAL("Fatal");

	NG_VERIFY(false, "First message");
	NG_VERIFY(false, "Second message, {0} {1} {2}", 1, 2, 3);

	NG_ASSERT(false, "Assert");

	NG_UNREACHABLE();
	return 0;
}