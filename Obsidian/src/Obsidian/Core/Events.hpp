#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <variant>
#include <concepts>
#include <functional>
#include <type_traits>

#include <Nano/Nano.hpp>

#include "Obsidian/Core/Input/KeyCodes.hpp"
#include "Obsidian/Core/Input/MouseCodes.hpp"

namespace Obsidian
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Windows events
    ////////////////////////////////////////////////////////////////////////////////////
    class WindowResizeEvent
    {
    public:
        // Constructors & Destructor
        inline constexpr WindowResizeEvent(uint32_t width, uint32_t height)
            : m_Width(width), m_Height(height) {}
        constexpr ~WindowResizeEvent() = default;

        // Getters
        inline constexpr uint32_t GetWidth() const { return m_Width; }
        inline constexpr uint32_t GetHeight() const { return m_Height; }

    private:
        uint32_t m_Width, m_Height;
    };

    class WindowCloseEvent
    {
    public:
        // Constructors & Destructor
        constexpr WindowCloseEvent() = default;
        constexpr ~WindowCloseEvent() = default;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Key events
    ////////////////////////////////////////////////////////////////////////////////////
    class KeyPressedEvent
    {
    public:
        // Constructors & Destructor
        inline constexpr KeyPressedEvent(int keycode, int repeatCount)
            : m_KeyCode(static_cast<Key>(keycode)), m_RepeatCount(repeatCount) {}
        constexpr ~KeyPressedEvent() = default;

        // Getters
        inline constexpr Key GetKeyCode() const { return m_KeyCode; }
        inline constexpr int GetRepeatCount() const { return m_RepeatCount; }

    private:
        Key m_KeyCode;
        int m_RepeatCount;
    };

    class KeyReleasedEvent
    {
    public:
        // Constructors & Destructor
        inline constexpr KeyReleasedEvent(int keycode)
            : m_KeyCode(static_cast<Key>(keycode)) {}
        constexpr ~KeyReleasedEvent() = default;

        // Getters
        inline constexpr Key GetKeyCode() const { return m_KeyCode; }

    private:
        Key m_KeyCode;
    };

    class KeyTypedEvent
    {
    public:
        // Constructors & Destructor
        inline constexpr KeyTypedEvent(int keycode)
            : m_KeyCode(static_cast<Key>(keycode)) {}
        constexpr ~KeyTypedEvent() = default;

        // Getters
        inline constexpr Key GetKeyCode() const { return m_KeyCode; }

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
        inline constexpr MouseMovedEvent(float x, float y)
            : m_MouseX(x), m_MouseY(y) {}
        constexpr ~MouseMovedEvent() = default;

        // Getters
        inline constexpr float GetX() const { return m_MouseX; }
        inline constexpr float GetY() const { return m_MouseY; }

    private:
        float m_MouseX, m_MouseY;
    };

    class MouseScrolledEvent
    {
    public:
        // Constructors & Destructor
        inline constexpr MouseScrolledEvent(float xOffset, float yOffset)
            : m_XOffset(xOffset), m_YOffset(yOffset) {}
        constexpr ~MouseScrolledEvent() = default;

        // Getters
        inline constexpr float GetXOffset() const { return m_XOffset; }
        inline constexpr float GetYOffset() const { return m_YOffset; }

    private:
        float m_XOffset, m_YOffset;
    };

    class MouseButtonPressedEvent
    {
    public:
        // Constructors & Destructor
        inline constexpr MouseButtonPressedEvent(int button)
            : m_Button(static_cast<MouseButton>(button)) {}
        constexpr ~MouseButtonPressedEvent() = default;

        // Getters
        inline constexpr MouseButton GetButtonCode() const { return m_Button; }

    private:
        MouseButton m_Button;
    };

    class MouseButtonReleasedEvent
    {
    public:
        // Constructors & Destructor
        inline constexpr MouseButtonReleasedEvent(int button)
            : m_Button(static_cast<MouseButton>(button)) {}
        constexpr ~MouseButtonReleasedEvent() = default;

        // Getters
        inline constexpr MouseButton GetButtonCode() const { return m_Button; }

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
