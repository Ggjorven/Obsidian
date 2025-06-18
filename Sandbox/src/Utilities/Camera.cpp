#include "Camera.hpp"

#include "NanoGraphics/Maths/Functions.hpp"

#include <Nano/Nano.hpp>

Camera::Camera(const Window& window)
    : m_Window(window)
{
    RecalculateProjectionMatrix();
}

Camera::~Camera()
{
}

void Camera::OnUpdate(float deltaTime)
{
    constexpr const float s_DeltaMovement = 0.001f;

    if (m_Window.GetInput().IsKeyPressed(Key::W))
    {
        m_Position.y += s_DeltaMovement * deltaTime;
    }
    if (m_Window.GetInput().IsKeyPressed(Key::A))
    {
        m_Position.x -= s_DeltaMovement * deltaTime;
    }
    if (m_Window.GetInput().IsKeyPressed(Key::S))
    {
        m_Position.y -= s_DeltaMovement * deltaTime;
    }
    if (m_Window.GetInput().IsKeyPressed(Key::D))
    {
        m_Position.x += s_DeltaMovement * deltaTime;
    }

    RecalculateViewMatrix();
}

void Camera::OnEvent(Event& e)
{
    Nano::Events::EventHandler handler(e);

    handler.Handle<MouseScrolledEvent>([this](MouseScrolledEvent& e) -> bool
    {
        constexpr const float s_BaseZoomSpeed = 0.25f;

        SetZoom(m_Zoom - (e.GetYOffset() * s_BaseZoomSpeed));
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
