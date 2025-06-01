#pragma once

#include "NanoGraphics/Core/WindowSpec.hpp"
//#include "NanoGraphics/Renderer/Renderer.hpp"

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
	class DesktopWindow
	{
	public:
		// Constructors & Destructor
		DesktopWindow(const WindowSpecification& specs, Window* instance);
		~DesktopWindow();

		// Methods
		void PollEvents();
		void SwapBuffers();

		void Resize(uint32_t width, uint32_t height);
		void Close();

		// Getters
		Graphics::Maths::Vec2<uint32_t> GetSize() const;
		Graphics::Maths::Vec2<int32_t> GetPosition() const;

		// Setters
		void SetTitle(std::string_view title);
		void SetVSync(bool vsync);

		// Additional getters
		inline bool IsVSync() const { return m_Specification.VSync; }
		inline bool IsOpen() const { return !m_Closed; }
		inline bool IsMinimized() const { return ((m_Specification.Width == 0) || (m_Specification.Height == 0)); }

		inline void* GetNativeWindow() { return static_cast<void*>(m_Window); }
		inline WindowSpecification& GetSpecification() { return m_Specification; }
		//inline Renderer& GetRenderer() { return m_Renderer.Get(); }

	private:
		WindowSpecification m_Specification;

		GLFWwindow* m_Window = nullptr;

		bool m_Closed = false;

		//DeferredConstruct<Renderer, true> m_Renderer = {};
	};
#endif

}