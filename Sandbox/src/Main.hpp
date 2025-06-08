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

		// Destroy prep
		std::queue<DeviceDestroyFn> destroyQueue = {};
		auto emptyQueue = [&]() { while (!destroyQueue.empty()) { destroyQueue.front()(); destroyQueue.pop(); } };

		// Device Creation
		DeviceSpecification deviceSpecs = DeviceSpecification()
			.SetNativeWindow(windowPtr->GetNativeWindow())
			.SetMessageCallback([](DeviceMessageType msgType, const std::string& message)
			{
				switch (msgType)
				{
				case DeviceMessageType::Error:
					NG_LOG_ERROR("Device Error: {0}", message);
					break;
				case DeviceMessageType::Warn:
					NG_LOG_WARN("Device Warning: {0}", message);
					break;

				default:
					break;
				}
			})
			.SetDestroyCallback([&](DeviceDestroyFn fn)
			{
				destroyQueue.push(fn);
			});
		Device device(deviceSpecs);

		// Swapchain
		SwapchainSpecification swapchainSpecs = SwapchainSpecification()
			.SetWindow(window)
			.SetFormat(Format::BGRA8Unorm)
			.SetColourSpace(ColourSpace::SRGB)
			.SetVSync(false);
		Swapchain swapchain = device.CreateSwapchain(swapchainSpecs);

		// Commandlists & Commandpool
		CommandListPool pool = device.AllocateCommandListPool({});
		CommandList list = pool.AllocateList({});

		// Main Loop
		while (window.IsOpen())
		{
			window.PollEvents();

			emptyQueue();
			swapchain.AcquireNextImage();

			list.Begin(true);
			list.End();
			list.Submit({ CommandQueue::Graphics });

			swapchain.Present();
		}

		device.DestroySwapchain(swapchain);
		emptyQueue();
	}

	return 0;
}