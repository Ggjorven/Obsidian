#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Window.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#include <Nano/Nano.hpp>

using namespace Nano;
using namespace Nano::Graphics;

int Main(int argc, char* argv[])
{
	{
		// Window Creation
		Window* windowPtr;
		WindowSpecification windowSpecs = WindowSpecification()
			.SetTitle("First")
			.SetWidthAndHeight(1280, 720)
			.SetFlags(WindowFlags::Default)
			.SetEventCallback([&](Event e) 
			{
				Events::EventHandler handler(e);
				handler.Handle<WindowCloseEvent>([&](WindowCloseEvent&) mutable { windowPtr->Close(); });
			});
		Window window(windowSpecs);
		windowPtr = &window;

		// Device Creation
		DeviceSpecification deviceSpecs = DeviceSpecification()
			.SetNativeWindow(windowPtr->GetNativeWindow())
			.SetMessageCallback([](DeviceMessage msgType, const std::string& message)
			{
				switch (msgType)
				{
				case DeviceMessage::Error:
					NG_LOG_ERROR("Device Error: {0}", message);
					break;
				case DeviceMessage::Warn:
					NG_LOG_WARN("Device Warning: {0}", message);
					break;

				default:
					break;
				}
			});
		Device device(deviceSpecs);

		// CommandPool & Lists
		CommandListPoolSpecification poolSpecs = CommandListPoolSpecification()
			.SetDebugName("First pool");
		CommandListPool pool = device.AllocateCommandListPool(poolSpecs);

		CommandListSpecification listSpecs = CommandListSpecification()
			.SetDebugName("First list");
		CommandList list = pool.AllocateList(listSpecs);

		pool.FreeList(list);
		device.FreePool(pool);

		// Image creation (!TEST!, should be in SwapChain but doesn't exist yet)
		ImageSpecification imageSpecs = ImageSpecification()
			.SetImageFormat(Format::BGRA8Unorm)
			.SetImageDimension(ImageDimension::Image2D)
			.SetWidthAndHeight(1280, 720)
			.SetDebugName("First image");
		Image image = device.CreateImage(imageSpecs);

		// Main Loop
		while (window.IsOpen())
		{
			window.PollEvents();
		}
	}

	return 0;
}