#pragma once

#include "NanoGraphics/Core/Input/KeyCodes.hpp"
#include "NanoGraphics/Core/Input/MouseCodes.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include <Nano/Nano.hpp>

#if defined(NG_PLATFORM_DESKTOP)
	#include <GLFW/glfw3.h>
#endif

#include <string>

namespace Nano::Graphics
{
	class Input;
}

namespace Nano::Graphics::Internal
{

	class DesktopInput;

#if defined(NG_PLATFORM_DESKTOP)
	////////////////////////////////////////////////////////////////////////////////////
	// DesktopInput
	////////////////////////////////////////////////////////////////////////////////////
	class DesktopInput : public Traits::NoCopy
	{
	public:
		// Constructors & Destructor
		DesktopInput() = default;
		~DesktopInput() = default;

		// Methods
		bool IsKeyPressed(Key keycode) const;
		bool IsMousePressed(MouseButton button) const;

		Nano::Graphics::Maths::Vec2<double> GetCursorPosition() const;
		void SetCursorPosition(Nano::Graphics::Maths::Vec2<double> position) const;
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