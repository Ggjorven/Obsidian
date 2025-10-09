#pragma once

#include "Obsidian/Core/Information.hpp"
#include "Obsidian/Core/Input/KeyCodes.hpp"
#include "Obsidian/Core/Input/MouseCodes.hpp"

#include "Obsidian/Maths/Structs.hpp"

#include <Nano/Nano.hpp>

#include <string>

namespace Obsidian
{
	class Input;
}

namespace Obsidian::Internal
{

	class DesktopInput;

#if defined(OB_PLATFORM_DESKTOP)
	////////////////////////////////////////////////////////////////////////////////////
	// DesktopInput
	////////////////////////////////////////////////////////////////////////////////////
	class DesktopInput
	{
	public:
		// Constructors & Destructor
		DesktopInput() = default;
		~DesktopInput() = default;

		// Methods
		bool IsKeyPressed(Key keycode) const;
		bool IsMousePressed(MouseButton button) const;

		Obsidian::Maths::Vec2<double> GetCursorPosition() const;
		void SetCursorPosition(Obsidian::Maths::Vec2<double> position) const;
		void SetCursorMode(CursorMode mode) const;

		std::string GetClipboard() const;
		void SetClipboard(const std::string& input) const;

		// Internal setters
		inline void SetWindow(void* window) { m_NativeWindow = window; }

	private:
		void* m_NativeWindow = nullptr;
	};
#endif

}