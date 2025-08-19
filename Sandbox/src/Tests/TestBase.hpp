#include "Obsidian/Core/Logging.hpp"
#include "Obsidian/Core/Window.hpp"

#include "Obsidian/Renderer/Device.hpp"

#include "Obsidian/Maths/Structs.hpp"

#include <Nano/Nano.hpp>

#include <cstdint>
#include <string_view>

using namespace Obsidian;

class TestBase
{
public:
	// Constructor & Destructor
	TestBase(uint32_t width, uint32_t height, std::string_view windowName, EventCallbackFn eventCallback, DeviceMessageCallback deviceCallback, Format format = Format::BGRA8Unorm, ColourSpace space = ColourSpace::SRGB)
	{
		// Window
		m_Window.Construct(WindowSpecification()
			.SetTitle(std::string(windowName))
			.SetWidthAndHeight(width, height)
			.SetFlags(WindowFlags::Resizable | WindowFlags::Decorated | WindowFlags::Visible | WindowFlags::Focused | WindowFlags::FocusOnShow)
			.SetEventCallback(eventCallback)
		);

		// Device
		m_Device.Construct(DeviceSpecification()
			.SetNativeWindow(m_Window->GetNativeWindow())
			.SetMessageCallback(deviceCallback)
			.SetDestroyCallback([this](DeviceDestroyFn fn) { m_DestroyQueue.push(fn); })
		);

		// Swapchain
		m_Swapchain.Construct(m_Device.Get(), SwapchainSpecification()
			.SetWindow(m_Window.Get())
			.SetFormat(format)
			.SetColourSpace(space)
#if defined(OB_PLATFORM_APPLE)
			.SetVSync(true) // Note: Vulkan via MoltenVK without VSync causes bad screen tearing.
#else
			.SetVSync(false)
#endif
			.SetDebugName("Swapchain")
		);
	}

	~TestBase()
	{
		m_Device->DestroySwapchain(m_Swapchain.Get());

		m_Device->Wait();
		FreeQueue();
	}

protected:
	// Private methods
	void FreeQueue()
	{
		while (!m_DestroyQueue.empty())
		{
			m_DestroyQueue.front()();
			m_DestroyQueue.pop();
		}
	}

	// Private getters
	double GetDeltaTime()
	{
		double time = m_Window->GetWindowTime();
		double deltaTime = time - m_LastTime;
		m_LastTime = time;
		return deltaTime;
	}

protected:
	Nano::Memory::DeferredConstruct<Window> m_Window = {};

	Nano::Memory::DeferredConstruct<Device> m_Device = {};
	Nano::Memory::DeferredConstruct<Swapchain> m_Swapchain = {};

	double m_LastTime = 0.0;

	std::queue<DeviceDestroyFn> m_DestroyQueue = {};
};