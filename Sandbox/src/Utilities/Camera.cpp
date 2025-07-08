#include "Camera.hpp"

#include "NanoGraphics/Maths/Functions.hpp"

#include <Nano/Nano.hpp>

Camera::Camera(const Window& window)
    : m_Window(window), m_MousePosition({ m_Window.GetInput().GetCursorPosition() })
{
    RecalculateProjectionMatrix();
}

Camera::~Camera()
{
}

void Camera::OnUpdate(float deltaTime)
{
    (void)deltaTime;

    constexpr const float s_MouseSensitivity = 0.002775f;

    if (m_Window.GetInput().IsMousePressed(MouseButton::Right))
    {
        Maths::Vec2<float> currentMousePosition = m_Window.GetInput().GetCursorPosition();

        if (m_WasMouseDown)
        {
            Maths::Vec2<float> mouseDelta = currentMousePosition - m_MousePosition;
            m_Position.x -= mouseDelta.x * (s_MouseSensitivity * m_Zoom);
            m_Position.y += mouseDelta.y * (s_MouseSensitivity * m_Zoom);
        }

        m_MousePosition = currentMousePosition;
        m_WasMouseDown = true;
    }
    else
    {
        m_WasMouseDown = false;
    }

    RecalculateViewMatrix();
}

void Camera::OnEvent(Event& e)
{
    Nano::Events::EventHandler handler(e);

    handler.Handle<MouseScrolledEvent>([this](MouseScrolledEvent& mse) -> bool
    {
        constexpr const float s_BaseZoomSpeed = 0.25f;

        SetZoom(m_Zoom - (mse.GetYOffset() * s_BaseZoomSpeed));
        return false;
    });
    handler.Handle<WindowResizeEvent>([this](WindowResizeEvent&) -> bool
    {
        RecalculateProjectionMatrix();
        return false;
    });
}

void Camera::SetPosition(const Maths::Vec3<float>& position)
{
    m_Position = position;
    RecalculateViewMatrix();
}

void Camera::SetZoom(float zoom)
{
    constexpr const float s_MinZoom = 0.25f;

    m_Zoom = std::max(s_MinZoom, zoom);
    RecalculateProjectionMatrix();
}

void Camera::RecalculateViewMatrix()
{
    Maths::Mat4<float> transform = Maths::Translate(Maths::Mat4<float>(1.0f), m_Position) * Maths::Rotate(Maths::Mat4<float>(1.0f), Maths::Radians(m_Rotation), Maths::Vec3<float>(0.0f, 0.0f, 1.0f));

    m_Camera.ViewMatrix = glm::inverse(transform);
}

void Camera::RecalculateProjectionMatrix()
{
    float aspectRatio = Maths::AspectRatio(m_Window.GetSize().x, m_Window.GetSize().y);

    m_Camera.ProjectionMatrix = Maths::Orthographic(aspectRatio, m_Zoom);
    m_Camera.ProjectionMatrix[1][1] *= -1.0f;
}