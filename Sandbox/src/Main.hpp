#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Window.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#include <Nano/Nano.hpp>

using namespace Nano;
using namespace Nano::Graphics;

int Main(int argc, char* argv[])
{
	(void)argc; (void)argv;

	{
		// Global pointers
		Window* windowPtr = nullptr;
		Swapchain* swapchainPtr = nullptr;
		Renderpass* renderpassPtr = nullptr;

		// Window Creation
		WindowSpecification windowSpecs = WindowSpecification()
			.SetTitle("First")
			.SetWidthAndHeight(1280, 720)
			.SetFlags(WindowFlags::Default)
			.SetEventCallback([&](Event e) 
			{
				Events::EventHandler handler(e);
				handler.Handle<WindowCloseEvent>([&](WindowCloseEvent&) mutable { windowPtr->Close(); });
				handler.Handle<WindowResizeEvent>([&](WindowResizeEvent& wre) mutable 
				{ 
					swapchainPtr->Resize(wre.GetWidth(), wre.GetHeight()); 
					renderpassPtr->ResizeFramebuffers();
				});
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
			.SetVSync(false)
			.SetDebugName("Swapchain");
		Swapchain swapchain = device.CreateSwapchain(swapchainSpecs);
		swapchainPtr = &swapchain;

		// Commandlists & Commandpool
		CommandListPool pool = swapchain.AllocateCommandListPool({ "First pool" });
		std::array<CommandList, Information::BackBufferCount> lists = {
			pool.AllocateList({ "First List" }),
			pool.AllocateList({ "Second List" }),
			pool.AllocateList({ "Third List" }),
		};

		// Renderpass & Framebuffers
		RenderpassSpecification renderpassSpecs = RenderpassSpecification()
			.SetBindpoint(PipelineBindpoint::Graphics)
			
			.SetColourImageSpecification(swapchain.GetImage(0).GetSpecification())
			.SetColourLoadOperation(LoadOperation::Clear)
			.SetColourStoreOperation(StoreOperation::Store)
			.SetColourStartState(ResourceState::Unknown)
			.SetColourEndState(ResourceState::Present);
		Renderpass renderpass = device.CreateRenderpass(renderpassSpecs);
		renderpassPtr = &renderpass;

		for (uint8_t i = 0; i < Information::BackBufferCount; i++)
		{
			FramebufferSpecification framebufferSpecs = FramebufferSpecification()
				.SetColourAttachment(FramebufferAttachment()
					.SetImage(swapchain.GetImage(i)));

			(void)renderpass.CreateFramebuffer(framebufferSpecs);
		}

		// Main Loop
		while (window.IsOpen())
		{
			if (window.IsFocused()) [[likely]]
				window.PollEvents();
			else
				window.WaitEvents();

			emptyQueue();
			swapchain.AcquireNextImage();

			CommandList& list = lists[swapchain.GetCurrentFrame()];
			
			{
				list.ResetAndOpen();
				{
					GraphicsState state = GraphicsState()
						.SetRenderpass(renderpass)
						.SetViewport(Viewport(static_cast<float>(window.GetSize().x), static_cast<float>(window.GetSize().y)))
						.SetScissor(ScissorRect(Viewport(static_cast<float>(window.GetSize().x), static_cast<float>(window.GetSize().y))))
						.SetColourClear({ 1.0f, 1.0f, 0.0f, 1.0f });
					list.SetGraphicsState(state);
				}
				list.Close();

				CommandListSubmitArgs args = CommandListSubmitArgs()
					.SetQueue(CommandQueue::Graphics)
					.SetWaitForSwapchainImage(true)
					.SetOnFinishMakeSwapchainPresentable(true);
				list.Submit(args);
			}
			swapchain.Present();
		}

		device.DestroyRenderpass(renderpass);

		swapchain.FreePool(pool);
		device.DestroySwapchain(swapchain);

		device.Wait();
		emptyQueue();
	}

	return 0;
}