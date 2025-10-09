#pragma once

#include "Obsidian/Core/Information.hpp"
#include "Obsidian/Core/WindowSpec.hpp"

#include "Obsidian/Platform/Desktop/DesktopWindow.hpp"

#include <Nano/Nano.hpp>

#include <cstdint>

namespace Obsidian
{

	////////////////////////////////////////////////////////////////////////////////////
	// Window
	////////////////////////////////////////////////////////////////////////////////////
	class Window
	{
	public:
		using Type = Nano::Types::SelectorType<Information::Platform,
			Nano::Types::EnumToType<Information::Structs::Platform::Windows,	Internal::DesktopWindow>,
			Nano::Types::EnumToType<Information::Structs::Platform::Linux,		Internal::DesktopWindow>,
			Nano::Types::EnumToType<Information::Structs::Platform::MacOS,		Internal::DesktopWindow>
		>;
	public:
		// Constructor & Destructor
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
		inline Obsidian::Maths::Vec2<uint32_t> GetSize() const { return m_Window.GetSize(); }
		inline Obsidian::Maths::Vec2<int32_t> GetPosition() const { return m_Window.GetPosition(); }

		inline double GetWindowTime() const { return m_Window.GetWindowTime(); } // Note: Time is in seconds

		// Setters
		inline void SetSize(uint32_t width, uint32_t height) { m_Window.SetSize(width, height); }
		inline void SetTitle(std::string_view title) { m_Window.SetTitle(title); }

		// Note: This is in format RGBA, 4 channels, arranged left-to-right, top-to-bottom.
		inline void SetIcon(uint32_t width, uint32_t height, const uint8_t* data) { m_Window.SetIcon(width, height, data); }

		// Additional getters
		inline bool IsOpen() const { return m_Window.IsOpen(); }
		inline bool IsFocused() const { return m_Window.IsFocused(); }
		inline bool IsMinimized() const { return m_Window.IsMinimized(); }

		inline void* GetNativeWindow() { return m_Window.GetNativeWindow(); }
		inline const WindowSpecification& GetSpecification() const { return m_Window.GetSpecification(); }

		inline const Input& GetInput() const { return m_Window.GetInput(); }

	private:
		Type m_Window;
	};

}