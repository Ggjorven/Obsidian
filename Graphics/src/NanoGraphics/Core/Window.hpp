#pragma once

#include <Nano/Nano.hpp>

#include "NanoGraphics/Core/WindowSpec.hpp"

#include "NanoGraphics/Platform/Desktop/DesktopWindow.hpp"

namespace Nano::Graphics
{

	////////////////////////////////////////////////////////////////////////////////////
	// Window
	////////////////////////////////////////////////////////////////////////////////////
	class Window
	{
	public:
		using Type = Types::SelectorType<CompileInformation::Platform,
			Types::EnumToType<CompileInformation::Structs::Platform::Windows, Internal::DesktopWindow>,
			Types::EnumToType<CompileInformation::Structs::Platform::Linux, Internal::DesktopWindow>,
			Types::EnumToType<CompileInformation::Structs::Platform::MacOS, Internal::DesktopWindow>
		>;
	public:
		// Constructor & Destructor
		Window() = default;
		inline Window(const WindowSpecification& specs)
			: m_Window(specs, this) {}
		~Window() = default;

		// Methods
		inline void PollEvents() { m_Window.PollEvents(); }
		inline void SwapBuffers() { m_Window.SwapBuffers(); }

		// Note: This is not resizing the window, it's resizing the drawing area (on the internal renderer).
		inline void Resize(uint32_t width, uint32_t height) { m_Window.Resize(width, height); }
		inline void Close() { m_Window.Close(); }

		// Getters
		inline Graphics::Maths::Vec2<uint32_t> GetSize() const { return m_Window.GetSize(); }
		inline Graphics::Maths::Vec2<int32_t> GetPosition() const { return m_Window.GetPosition(); }

		// Setters
		inline void SetTitle(std::string_view title) { m_Window.SetTitle(title); }
		inline void SetVSync(bool vsync) { m_Window.SetVSync(vsync); }

		// Additional getters
		inline bool IsVSync() const { return m_Window.IsVSync(); }
		inline bool IsOpen() const { return m_Window.IsOpen(); }
		inline bool IsMinimized() const { return m_Window.IsMinimized(); }

		inline void* GetNativeWindow() { return m_Window.GetNativeWindow(); }
		inline WindowSpecification& GetSpecification() { return m_Window.GetSpecification(); }
		//inline Renderer& GetRenderer() { return m_Window.GetRenderer(); }

	private:
		Type m_Window;
	};

}