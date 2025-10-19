#include "obpch.h"
#include "DesktopWindow.hpp"

#include "Obsidian/Core/Logging.hpp"
#include "Obsidian/Utils/Profiler.hpp"

namespace Obsidian::Internal
{

#if defined(OB_PLATFORM_DESKTOP)
    namespace
    {
        static uint8_t s_GLFWInstances = 0;

        static void GLFWErrorCallBack(int errorCode, const char* description)
        {
#if !defined(OB_CONFIG_DIST)
            OB_LOG_ERROR("[GLFW]: ({0}), {1}", errorCode, description);
#else
            (void)errorCode; (void)description;
#endif
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructors & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    DesktopWindow::DesktopWindow(const WindowSpecification& specs)
        : m_Specification(specs)
    {
        OB_ASSERT(!specs.Title.empty(), "[DesktopWindow] No title passed in.");
        OB_ASSERT(((specs.Width != 0) && (specs.Height != 0)), "[DesktopWindow] Invalid width & height passed in.");
        OB_ASSERT(static_cast<bool>(specs.EventCallback), "[DesktopWindow] No event callback passed in.");

        // Initialize windowing library
        if (s_GLFWInstances == 0)
        {
            int result = glfwInit();
            OB_ASSERT((result), "[DesktopWindow] Failed to initialize windowing library.");

            s_GLFWInstances++;
            glfwSetErrorCallback(GLFWErrorCallBack);
        }

        // Set user set flags
        glfwWindowHint(GLFW_RESIZABLE, static_cast<bool>(specs.Flags & WindowFlags::Resizable));
        // Note: Decorated must be set after the window is created: https://github.com/glfw/glfw/issues/2060
        glfwWindowHint(GLFW_VISIBLE, static_cast<bool>(specs.Flags & WindowFlags::Visible));
        glfwWindowHint(GLFW_MAXIMIZED, static_cast<bool>(specs.Flags & WindowFlags::Maximized));
        glfwWindowHint(GLFW_FLOATING, static_cast<bool>(specs.Flags & WindowFlags::Floating));
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, static_cast<bool>(specs.Flags & WindowFlags::Transparent));
        glfwWindowHint(GLFW_FOCUSED, static_cast<bool>(specs.Flags & WindowFlags::Focused));
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, static_cast<bool>(specs.Flags & WindowFlags::FocusOnShow));
        glfwWindowHint(GLFW_CENTER_CURSOR, static_cast<bool>(specs.Flags & WindowFlags::CenterCursor));

        // Create the window
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_Window = glfwCreateWindow(static_cast<int>(specs.Width), static_cast<int>(specs.Height), specs.Title.c_str(), nullptr, nullptr);
        OB_ASSERT(m_Window, "[DesktopWindow] Failed to create a window.");

        m_Input.SetWindow(m_Window); // Init Input

        glfwSetWindowAttrib(m_Window, GLFW_DECORATED, static_cast<bool>(specs.Flags & WindowFlags::Decorated));

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
    void DesktopWindow::WaitEvents()
    {
        glfwWaitEvents();
    }

    void DesktopWindow::WaitEvents(double timeout)
    {
        glfwWaitEventsTimeout(timeout);
    }

    void DesktopWindow::SwapBuffers()
    {
        // Note: Currently the renderer handles presenting to the screen using Swapchain::Present()
        // This function exists for future API's like OpenGL which require a seperate swapbuffers call
    }

    void DesktopWindow::Show()
    {
        glfwShowWindow(m_Window);
    }

    void DesktopWindow::SetFocus()
    {
        glfwFocusWindow(m_Window);
    }

    void DesktopWindow::Close()
    {
        m_Closed = true;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Static methods
    ////////////////////////////////////////////////////////////////////////////////////
    void DesktopWindow::PollEvents()
    {
        OB_MARK_FRAME();
        OB_PROFILE("DesktopWindow::PollEvents()");
        glfwPollEvents();
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Getters
    ////////////////////////////////////////////////////////////////////////////////////
    Obsidian::Maths::Vec2<uint32_t> DesktopWindow::GetSize() const
    {
        return { m_Specification.Width, m_Specification.Height };
    }

    Obsidian::Maths::Vec2<int32_t> DesktopWindow::GetPosition() const
    {
        OB_PROFILE("DesktopWindow::GetPosition()");

        Obsidian::Maths::Vec2<int32_t> position = { 0, 0 };
        glfwGetWindowPos(m_Window, &position.x, &position.y);
        return position;
    }

    double DesktopWindow::GetWindowTime() const
    {
        return glfwGetTime();
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Setters
    ////////////////////////////////////////////////////////////////////////////////////
    void DesktopWindow::SetSize(uint32_t width, uint32_t height)
    {
        OB_PROFILE("DesktopWindow::SetSize()");
        glfwSetWindowSize(m_Window, static_cast<int>(width), static_cast<int>(height));
    }

    void DesktopWindow::SetTitle(std::string_view title)
    {
        OB_PROFILE("DesktopWindow::SetTitle()");

        m_Specification.Title = title;
        glfwSetWindowTitle(m_Window, m_Specification.Title.data());
    }

    void DesktopWindow::SetIcon(uint32_t width, uint32_t height, const uint8_t* data)
    {
        GLFWimage image = {};
        image.width = static_cast<int>(width);
        image.height = static_cast<int>(height);
        image.pixels = const_cast<uint8_t*>(data);

        glfwSetWindowIcon(m_Window, 1, &image);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Getters
    ////////////////////////////////////////////////////////////////////////////////////
    bool DesktopWindow::IsFocused() const
    {
        return static_cast<bool>(glfwGetWindowAttrib(m_Window, GLFW_FOCUSED));
    }
#endif

}