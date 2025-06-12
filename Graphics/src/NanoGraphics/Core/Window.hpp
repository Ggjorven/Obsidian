#pragma once

#include "NanoGraphics/Core/WindowSpec.hpp"

#include "NanoGraphics/Platform/Desktop/DesktopWindow.hpp"

#include <Nano/Nano.hpp>

#include <cstdint>

namespace Nano::Graphics
{

	////////////////////////////////////////////////////////////////////////////////////
	// Window
	////////////////////////////////////////////////////////////////////////////////////
	class Window : public Traits::NoCopy
	{
	public:
		using Type = Types::SelectorType<CompileInformation::Platform,
			Types::EnumToType<CompileInformation::Structs::Platform::Windows,	Internal::DesktopWindow>,
			Types::EnumToType<CompileInformation::Structs::Platform::Linux,		Internal::DesktopWindow>,
			Types::EnumToType<CompileInformation::Structs::Platform::MacOS,		Internal::DesktopWindow>
		>;
	public:
		// Constructor & Destructor
		Window() = default;
		inline Window(const WindowSpecification& specs)
			: m_Window(specs) {}
		~Window() = default;

		// Methods
		inline void PollEvents() { m_Window.PollEvents(); }
		inline void WaitEvents() { m_Window.WaitEvents(); }
		inline void WaitEvents(double timeout) { m_Window.WaitEvents(timeout); } // Note: Timeout is in seconds
		inline void SwapBuffers() { m_Window.SwapBuffers(); }

		inline void Show() { m_Window.Show(); }
		inline void SetFocus() { m_Window.SetFocus(); }
		inline void Close() { m_Window.Close(); }

		// Getters
		inline Graphics::Maths::Vec2<uint32_t> GetSize() const { return m_Window.GetSize(); }
		inline Graphics::Maths::Vec2<int32_t> GetPosition() const { return m_Window.GetPosition(); }

		// Setters
		inline void SetSize(uint32_t width, uint32_t height) { m_Window.SetSize(width, height); }
		inline void SetTitle(std::string_view title) { m_Window.SetTitle(title); }

		// Additional getters
		inline bool IsOpen() const { return m_Window.IsOpen(); }
		inline bool IsFocused() const { return m_Window.IsFocused(); }
		inline bool IsMinimized() const { return m_Window.IsMinimized(); }

		inline void* GetNativeWindow() { return m_Window.GetNativeWindow(); }
		inline WindowSpecification& GetSpecification() { return m_Window.GetSpecification(); }

	private:
		Type m_Window;
	};

}