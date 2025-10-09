#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Core/Input/KeyCodes.hpp"
#include "Obsidian/Core/Input/MouseCodes.hpp"

#include "Obsidian/Maths/Structs.hpp"

#include "Obsidian/Platform/Desktop/DesktopInput.hpp"

#include <Nano/Nano.hpp>

#include <string>

namespace Obsidian
{

	////////////////////////////////////////////////////////////////////////////////////
	// Input
	////////////////////////////////////////////////////////////////////////////////////
	class Input
	{
	public:
		using Type = Nano::Types::SelectorType<Information::Platform,
			Nano::Types::EnumToType<Information::Structs::Platform::Windows, Internal::DesktopInput>,
			Nano::Types::EnumToType<Information::Structs::Platform::Linux, Internal::DesktopInput>,
			Nano::Types::EnumToType<Information::Structs::Platform::MacOS, Internal::DesktopInput>
		>;
	public:
		// Constructor & Destructor
		Input() = default;
		~Input() = default;

		// Methods
		inline bool IsKeyPressed(Key keycode) const { return m_Input.IsKeyPressed(keycode); }
		inline bool IsMousePressed(MouseButton button) const { return m_Input.IsMousePressed(button); }

		inline Maths::Vec2<double> GetCursorPosition() const  { return m_Input.GetCursorPosition(); }
		inline void SetCursorPosition(Maths::Vec2<double> position) const { m_Input.SetCursorPosition(position); }

		inline void SetCursorMode(CursorMode mode) const { m_Input.SetCursorMode(mode); }

		inline std::string GetClipboard() const { return m_Input.GetClipboard(); }
		inline void SetClipboard(const std::string& input) const { m_Input.SetClipboard(input); }

		// Internal setters
		inline void SetWindow(void* window) { m_Input.SetWindow(window); }

	private:
		Type m_Input;
	};

}
