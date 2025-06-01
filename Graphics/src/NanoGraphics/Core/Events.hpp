#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <variant>
#include <concepts>
#include <functional>
#include <type_traits>

#include <Nano/Nano.hpp>

#include "NanoGraphics/Core/Input/KeyCodes.hpp"
#include "NanoGraphics/Core/Input/MouseCodes.hpp"

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Windows events
    ////////////////////////////////////////////////////////////////////////////////////
    class WindowResizeEvent
    {
    public:
        // Constructors & Destructor
        inline WindowResizeEvent(uint32_t width, uint32_t height)
            : m_Width(width), m_Height(height) {}
        ~WindowResizeEvent() = default;

        // Getters
        inline uint32_t GetWidth() const { return m_Width; }
        inline uint32_t GetHeight() const { return m_Height; }

    private:
        uint32_t m_Width, m_Height;
    };

    class WindowCloseEvent
    {
    public:
        // Constructors & Destructor
        WindowCloseEvent() = default;
        ~WindowCloseEvent() = default;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Key events
    ////////////////////////////////////////////////////////////////////////////////////
    class KeyPressedEvent
    {
    public:
        // Constructors & Destructor
        inline KeyPressedEvent(int keycode, int repeatCount)
            : m_KeyCode(static_cast<Key>(keycode)), m_RepeatCount(repeatCount) {}
        ~KeyPressedEvent() = default;

        // Getters
        inline Key GetKeyCode() const { return m_KeyCode; }
        inline int GetRepeatCount() const { return m_RepeatCount; }

    private:
        Key m_KeyCode;
        int m_RepeatCount;
    };

    class KeyReleasedEvent
    {
    public:
        // Constructors & Destructor
        inline KeyReleasedEvent(int keycode)
            : m_KeyCode(static_cast<Key>(keycode)) {}
        ~KeyReleasedEvent() = default;

        // Getters
        inline Key GetKeyCode() const { return m_KeyCode; }

    private:
        Key m_KeyCode;
    };

    class KeyTypedEvent
    {
    public:
        // Constructors & Destructor
        inline KeyTypedEvent(int keycode)
            : m_KeyCode(static_cast<Key>(keycode)) {}
        ~KeyTypedEvent() = default;

        // Getters
        Key GetKeyCode() const { return m_KeyCode; }

    private:
        Key m_KeyCode;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Mouse events
    ////////////////////////////////////////////////////////////////////////////////////
    class MouseMovedEvent
    {
    public:
        // Constructors & Destructor
        inline MouseMovedEvent(float x, float y)
            : m_MouseX(x), m_MouseY(y) {}
        ~MouseMovedEvent() = default;

        // Getters
        inline float GetX() const { return m_MouseX; }
        inline float GetY() const { return m_MouseY; }

    private:
        float m_MouseX, m_MouseY;
    };

    class MouseScrolledEvent
    {
    public:
        // Constructors & Destructor
        inline MouseScrolledEvent(float xOffset, float yOffset)
            : m_XOffset(xOffset), m_YOffset(yOffset) {}
        ~MouseScrolledEvent() = default;

        // Getters
        inline float GetXOffset() const { return m_XOffset; }
        inline float GetYOffset() const { return m_YOffset; }

    private:
        float m_XOffset, m_YOffset;
    };

    class MouseButtonPressedEvent
    {
    public:
        // Constructors & Destructor
        inline MouseButtonPressedEvent(int button)
            : m_Button(static_cast<MouseButton>(button)) {}
        ~MouseButtonPressedEvent() = default;

        // Getters
        inline MouseButton GetButtonCode() const { return m_Button; }

    private:
        MouseButton m_Button;
    };

    class MouseButtonReleasedEvent
    {
    public:
        // Constructors & Destructor
        inline MouseButtonReleasedEvent(int button)
            : m_Button(static_cast<MouseButton>(button)) {}
        ~MouseButtonReleasedEvent() = default;

        // Getters
        inline MouseButton GetButtonCode() const { return m_Button; }

    private:
        MouseButton m_Button;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Event
    ////////////////////////////////////////////////////////////////////////////////////
    using Event = std::variant<
        WindowResizeEvent, WindowCloseEvent,
        KeyPressedEvent, KeyReleasedEvent, KeyTypedEvent,
        MouseMovedEvent, MouseScrolledEvent, MouseButtonPressedEvent, MouseButtonReleasedEvent
    >;

}
