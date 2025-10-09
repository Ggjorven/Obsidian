#include "Camera3D.hpp"

#include "Obsidian/Maths/Functions.hpp"

#include <Nano/Nano.hpp>

////////////////////////////////////////////////////////////////////////////////////
// Constructor & Destructor
////////////////////////////////////////////////////////////////////////////////////
Camera3D::Camera3D(const Window& window)
    : m_Window(window)
{
}

Camera3D::~Camera3D()
{
}

////////////////////////////////////////////////////////////////////////////////////
// Methods
////////////////////////////////////////////////////////////////////////////////////
void Camera3D::OnUpdate(float deltaTime)
{
    (void)deltaTime;

    Maths::Vec2<uint32_t> size = m_Window.GetSize();

    if (size.x != 0 && size.y != 0)
    {
        Maths::Vec2<double> position = m_Window.GetInput().GetCursorPosition();

        if (m_Window.GetInput().IsMousePressed(MouseButton::Right))
        {
            Maths::Vec2<float> delta = position - m_LastPosition;

            float deltaTheta = delta.x * m_Speed;
            float deltaPhi = delta.y * m_Speed;

            m_Theta -= deltaTheta;
            m_Phi += deltaPhi;

            constexpr static float epsilon = 0.001f; // Note: To prevent the camera from flipping
            m_Phi = Maths::Clamp(m_Phi, -Maths::HalfPi<float>() + epsilon, Maths::HalfPi<float>() - epsilon);
        }

        m_LastPosition = position;

        m_Position = Maths::Vec3<float>(
            m_Radius * Maths::Sin(m_Theta) * Maths::Cos(m_Phi),
            m_Radius * Maths::Sin(m_Phi),
            m_Radius * Maths::Cos(m_Theta) * Maths::Cos(m_Phi)
        );

        m_Camera3D.ViewMatrix = Maths::LookAt(m_Position, Maths::Vec3<float>(0.0f, 0.0f, 0.0f), Maths::Vec3<float>(0.0f, 1.0f, 0.0f));

        m_Camera3D.ProjectionMatrix = Maths::Perspective(Maths::Radians(m_FOV), static_cast<float>(size.x) / static_cast<float>(size.y), m_Near, m_Far);
        m_Camera3D.ProjectionMatrix = Maths::ApplyProjectionCorrection(m_Camera3D.ProjectionMatrix);
    }
}

void Camera3D::OnEvent(Event& e)
{
    Nano::Events::EventHandler handler(e);

    handler.Handle<MouseScrolledEvent>([this](MouseScrolledEvent& mse) -> bool
    {
        float change = m_Change * mse.GetYOffset();
        m_Radius -= change;
        return false;
    });
}