#include "ngpch.h"
#include "DesktopWindow.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

namespace Nano::Graphics::Internal
{

#if defined(NG_PLATFORM_DESKTOP)
    namespace
    {
        static uint8_t s_GLFWInstances = 0;

        static void GLFWErrorCallBack(int errorCode, const char* description)
        {
            NG_LOG_ERROR("[GLFW]: ({0}), {1}", errorCode, description);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructors & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    DesktopWindow::DesktopWindow(const WindowSpecification& specs)
        : m_Specification(specs)
    {
        NG_ASSERT(!specs.Title.empty(), "[DesktopWindow] No title passed in.");
        NG_ASSERT(((specs.Width != 0) && (specs.Height != 0)), "[DesktopWindow] Invalid width & height passed in.");
        NG_ASSERT(static_cast<bool>(specs.EventCallback), "[DesktopWindow] No event callback passed in.");

        // Initialize windowing library
        if (s_GLFWInstances == 0)
        {
            int result = glfwInit();
            NG_ASSERT((result), "[DesktopWindow] Failed to initialize windowing library.");

            s_GLFWInstances++;
            glfwSetErrorCallback(GLFWErrorCallBack);
        }

        // Create the window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_Window = glfwCreateWindow(static_cast<int>(specs.Width), static_cast<int>(specs.Height), specs.Title.data(), nullptr, nullptr);
        NG_ASSERT(m_Window, "[DesktopWindow] Failed to create a window.");

        // Making sure we can access the data in the callbacks
        glfwSetWindowUserPointer(m_Window, (void*)&m_Specification);

        // Setting the callbacks
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
        {
            WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);
            data.Width = width;
            data.Height = height;

            WindowResizeEvent event = WindowResizeEvent(width, height);
            data.EventCallback(event);
        });
        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
        {
            WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

            WindowCloseEvent event = WindowCloseEvent();
            data.EventCallback(event);
        });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int, int action, int)
        {
            WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

            switch (action)
            {
            case GLFW_PRESS:
            {
                KeyPressedEvent event = KeyPressedEvent(key, 0);
                data.EventCallback(event);
                break;
            }
            case GLFW_RELEASE:
            {
                KeyReleasedEvent event = KeyReleasedEvent(key);
                data.EventCallback(event);
                break;
            }
            case GLFW_REPEAT:
            {
                KeyPressedEvent event = KeyPressedEvent(key, 1);
                data.EventCallback(event);
                break;
            }
            }
        });
        glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
        {
            WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

            KeyTypedEvent event = KeyTypedEvent(keycode);
            data.EventCallback(event);
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int)
        {
            WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

            switch (action)
            {
            case GLFW_PRESS:
            {
                MouseButtonPressedEvent event = MouseButtonPressedEvent(button);
                data.EventCallback(event);
                break;
            }
            case GLFW_RELEASE:
            {
                MouseButtonReleasedEvent event = MouseButtonReleasedEvent(button);
                data.EventCallback(event);
                break;
            }
            }
        });
        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
        {
            WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

            MouseScrolledEvent event = MouseScrolledEvent((float)xOffset, (float)yOffset);
            data.EventCallback(event);
        });
        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
        {
            WindowSpecification& data = *(WindowSpecification*)glfwGetWindowUserPointer(window);

            MouseMovedEvent event = MouseMovedEvent((float)xPos, (float)yPos);
            data.EventCallback(event);
        });
    }

    DesktopWindow::~DesktopWindow()
    {
        m_Closed = true;

        glfwDestroyWindow(m_Window);
        m_Window = nullptr;

        if (--s_GLFWInstances == 0)
        {
            glfwTerminate();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void DesktopWindow::PollEvents()
    {
        NG_MARK_FRAME();
        NG_PROFILE("DesktopWindow::PollEvents()");
        glfwPollEvents();
    }

    void DesktopWindow::SwapBuffers()
    {
        NG_PROFILE("DesktopWindow::SwapBuffers()");
        NG_ASSERT(false, "[DesktopWindow] Unimplemented, maybe needs to be removed.");
    }

    void DesktopWindow::Close()
    {
        m_Closed = true;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Getters
    ////////////////////////////////////////////////////////////////////////////////////
    Graphics::Maths::Vec2<uint32_t> DesktopWindow::GetSize() const 
    { 
        NG_PROFILE("DesktopWindow::GetSize()");

        return { m_Specification.Width, m_Specification.Height }; 
    }

    Graphics::Maths::Vec2<int32_t> DesktopWindow::GetPosition() const
    {
        NG_PROFILE("DesktopWindow::GetPosition()");

        Graphics::Maths::Vec2<int32_t> position = { 0, 0 };
        glfwGetWindowPos(m_Window, &position.x, &position.y);
        return position;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Setters
    ////////////////////////////////////////////////////////////////////////////////////
    void DesktopWindow::SetSize(uint32_t width, uint32_t height)
    {
        NG_PROFILE("DesktopWindow::SetSize()");
        glfwSetWindowSize(m_Window, static_cast<int>(width), static_cast<int>(height));
    }

    void DesktopWindow::SetTitle(std::string_view title)
    {
        NG_PROFILE("DesktopWindow::SetTitle()");

        m_Specification.Title = title;
        glfwSetWindowTitle(m_Window, m_Specification.Title.data());
    }
#endif

}