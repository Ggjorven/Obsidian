#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Window.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include <Nano/Nano.hpp>

#include <string_view>

using namespace Nano::Graphics;

inline constexpr const std::string_view g_Fragment = R"(
// Simple HLSL fragment shader

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET
{
    // Gradient: red on the left, blue on the right
    float red  = 1.0 - input.uv.x;
    float blue = input.uv.x;
    return float4(red, 0.0, blue, 1.0);
}
)";

class Application
{
public:
	// Constructor & Destructor
	Application()
	{
		// Window
		m_Window.Construct(WindowSpecification()
			.SetTitle("First")
			.SetWidthAndHeight(1280, 720)
			.SetFlags(WindowFlags::Resizable | WindowFlags::Decorated | WindowFlags::Visible | WindowFlags::Focused | WindowFlags::FocusOnShow)
			.SetEventCallback([this](Event e) { OnEvent(e); })
		);

		// Device
		m_Device.Construct(DeviceSpecification()
			.SetNativeWindow(m_Window->GetNativeWindow())
			.SetMessageCallback([this](DeviceMessageType msgType, const std::string& message) { OnDeviceMessage(msgType, message); })
			.SetDestroyCallback([this](DeviceDestroyFn fn) { m_DestroyQueue.push(fn); })
		);

		// Swapchain
		m_Swapchain.Construct(m_Device.Get(), SwapchainSpecification()
			.SetWindow(m_Window.Get())
			.SetFormat(Format::BGRA8Unorm)
			.SetColourSpace(ColourSpace::SRGB)
			.SetVSync(false)
			.SetDebugName("Swapchain")
		);

		// Commandpools & Commandlists
		for (auto& pool : m_CommandPools)
			pool.Construct(m_Swapchain.Get(), CommandListPoolSpecification()
				.SetQueue(CommandQueue::Graphics)
				.SetDebugName("CommandPool")
			);

		for (size_t i = 0; i < m_CommandLists.size(); i++)
		{
			m_CommandLists[i].Construct(m_CommandPools[i].Get(), CommandListSpecification()
				.SetDebugName(std::format("CommandList for: {0}", m_CommandPools[i]->GetSpecification().DebugName))
			);
		}
	}
	~Application()
	{
		for (size_t i = 0; i < m_CommandPools.size(); i++)
		{
			m_CommandPools[i]->FreeList(m_CommandLists[i].Get());
			m_Swapchain->FreePool(m_CommandPools[i].Get());
		}

		m_Device->DestroySwapchain(m_Swapchain.Get());

		m_Device->Wait();
		FreeQueue();
	}

	// Methods
	void Run()
	{
		double lastTime = 0.0;

		while (m_Window->IsOpen())
		{
			if (m_Window->IsFocused()) [[likely]]
				m_Window->PollEvents();
			else
				m_Window->WaitEvents(1.0); // Note: When the windows is out of focus it only updates every second

			FreeQueue();

			double time = m_Window->GetWindowTime(); // Note: We use m_Window->GetWindowTime() instead of Nano's dedicated timer class because steadyclock on MacOS is very weird and unstable.
			Update(static_cast<float>(time - lastTime));
			lastTime = time;

			m_Swapchain->AcquireNextImage();
			{
				m_CommandPools[m_Swapchain->GetCurrentFrame()]->Reset();
				auto& list = m_CommandLists[m_Swapchain->GetCurrentFrame()];

				list->Open();

				list->Close();
				list->Submit(CommandListSubmitArgs()
					.SetWaitForSwapchainImage(true)
					.SetOnFinishMakeSwapchainPresentable(true)
				);
			}
			m_Swapchain->Present();
		}
	}

private:
	// Private methods
	void OnEvent(Event& e)
	{
		Nano::Events::EventHandler handler(e);
		handler.Handle<WindowCloseEvent>([&](WindowCloseEvent&) mutable { m_Window->Close(); });
		handler.Handle<WindowResizeEvent>([&](WindowResizeEvent& wre) mutable
		{
			m_Swapchain->Resize(wre.GetWidth(), wre.GetHeight());
		});
	}

	void OnDeviceMessage(DeviceMessageType msgType, const std::string& message)
	{
		switch (msgType)
		{
		case DeviceMessageType::Warn:
			NG_LOG_WARN("Device Warning: {0}", message);
			break;
		case DeviceMessageType::Error:
			NG_LOG_ERROR("Device Error: {0}", message);
			break;

		default:
			break;
		}
	}

	void FreeQueue()
	{
		while (!m_DestroyQueue.empty())
		{
			m_DestroyQueue.front()();
			m_DestroyQueue.pop();
		}
	}

	void Update(float deltaTime)
	{
	}

private:
	Nano::Memory::DeferredConstruct<Window> m_Window = {};

	Nano::Memory::DeferredConstruct<Device> m_Device = {};
	Nano::Memory::DeferredConstruct<Swapchain> m_Swapchain = {};

	std::array<Nano::Memory::DeferredConstruct<CommandListPool>, Information::FramesInFlight> m_CommandPools = { };
	std::array<Nano::Memory::DeferredConstruct<CommandList>, Information::FramesInFlight> m_CommandLists = { };

	std::queue<DeviceDestroyFn> m_DestroyQueue = {};
};

int Main(int argc, char* argv[])
{
	(void)argc; (void)argv;

	Application app;
	app.Run();
	return 0;
}