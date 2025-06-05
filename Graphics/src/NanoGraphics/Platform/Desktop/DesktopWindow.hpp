#pragma once

#include "NanoGraphics/Core/WindowSpec.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#if defined(NG_PLATFORM_DESKTOP)
	#include <GLFW/glfw3.h>
#endif

namespace Nano::Graphics
{
	class Window;
}

namespace Nano::Graphics::Internal
{

	class DesktopWindow;

#if defined(NG_PLATFORM_DESKTOP)
	////////////////////////////////////////////////////////////////////////////////////
	// DesktopWindow
	////////////////////////////////////////////////////////////////////////////////////
	class DesktopWindow : public Traits::NoCopy
	{
	public:
		// Constructors & Destructor
		DesktopWindow(const WindowSpecification& specs);
		~DesktopWindow();

		// Methods
		void PollEvents();
		void SwapBuffers();

		void Show();
		void SetFocus();
		void Close();

		// Getters
		Graphics::Maths::Vec2<uint32_t> GetSize() const;
		Graphics::Maths::Vec2<int32_t> GetPosition() const;

		// Setters
		void SetSize(uint32_t width, uint32_t height);
		void SetTitle(std::string_view title);

		// Additional getters
		inline bool IsOpen() const { return !m_Closed; }
		bool IsFocused() const;
		inline bool IsMinimized() const { return ((m_Specification.Width == 0) || (m_Specification.Height == 0)); }

		inline void* GetNativeWindow() { return static_cast<void*>(m_Window); }
		inline WindowSpecification& GetSpecification() { return m_Specification; }

	private:
		WindowSpecification m_Specification;

		GLFWwindow* m_Window = nullptr;

		bool m_Closed = false;
	};
#endif

}