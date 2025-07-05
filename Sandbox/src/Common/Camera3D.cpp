#include "Camera3D.hpp"

#include "NanoGraphics/Maths/Functions.hpp"

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
    switch (m_State)
    {
    case State::ArcBall:
        UpdateArcBall(deltaTime);
        break;
    case State::FlyCam:
        UpdateFlyCam(deltaTime);
        break;

    default:
        NG_LOG_ERROR("No proper camera state selected");
        break;
    }
}

void Camera3D::OnEvent(Event& e)
{
    Nano::Events::EventHandler handler(e);

    handler.Handle<MouseScrolledEvent>([this](MouseScrolledEvent& mse) -> bool
    {
        OnMouseScroll(mse);
        return false;
    });
}

void Camera3D::SwitchState()
{
    switch (m_State)
    {
    case State::ArcBall:
    {

        m_State = State::FlyCam;
        
        // Reset camera
        glm::vec3 direction = glm::normalize(glm::vec3(0.0f) - m_Position);
        m_Yaw = glm::degrees(atan2(direction.z, direction.x));
        m_Pitch = glm::degrees(asin(direction.y));
        break;
    }
    case State::FlyCam:
    {

        m_State = State::ArcBall;
        break;
    }

    default:
        NG_LOG_ERROR("No proper camera state selected");
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////
// Private methods
////////////////////////////////////////////////////////////////////////////////////
void Camera3D::UpdateArcBall(float deltaTime)
{
    if (m_Window.GetSize().x != 0 && m_Window.GetSize().y != 0)
    {
        static float theta = 0.0f;
        static float phi = 0.0f;

        static glm::vec2 lastPosition = { 0.0f, 0.0f };
        glm::vec2 position = m_Window.GetInput().GetCursorPosition();

        if (m_Window.GetInput().IsMousePressed(MouseButton::Right))
        {
            glm::vec2 delta = position - lastPosition;

            float dTheta = delta.x * m_Speed;
            float dPhi = delta.y * m_Speed;

            theta -= dTheta;
            phi += dPhi;

            float epsilon = 0.001f; // To prevent the camera from flipping
            phi = glm::clamp(phi, -glm::half_pi<float>() + epsilon, glm::half_pi<float>() - epsilon);
        }

        lastPosition = position;

        m_Position = glm::vec3(
            m_Radius * glm::sin(theta) * glm::cos(phi),
            m_Radius * glm::sin(phi),
            m_Radius * glm::cos(theta) * glm::cos(phi)
        );

        m_Camera3DData.ViewMatrix = glm::lookAt(m_Position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        m_Camera3DData.ProjectionMatrix = glm::perspective(glm::radians(m_FOV), (float)m_Window.GetSize().x / (float)m_Window.GetSize().y, m_Near, m_Far);
            m_Camera3DData.ProjectionMatrix[1][1] *= -1;
    }
}

void Camera3D::UpdateFlyCam(float deltaTime)
{
    if (m_Window.GetSize().x != 0 && m_Window.GetSize().y != 0)
    {
        if (m_Window.GetInput().IsMousePressed(MouseButton::Right))
        {
            float velocity = m_MovementSpeed * deltaTime;
            glm::vec3 moveDirection = { 0.0f, 0.0f, 0.0f };

            // Calculate forward/backward and left/right movement.
            if (m_Window.GetInput().IsKeyPressed(Key::W))
                moveDirection += m_Front;
            if (m_Window.GetInput().IsKeyPressed(Key::S))
                moveDirection -= m_Front;
            if (m_Window.GetInput().IsKeyPressed(Key::A))
                moveDirection -= m_Right;
            if (m_Window.GetInput().IsKeyPressed(Key::D))
                moveDirection += m_Right;

            // Calculate up/down movement.
            if (m_Window.GetInput().IsKeyPressed(Key::Space))
                moveDirection += m_Up;
            if (m_Window.GetInput().IsKeyPressed(Key::LeftShift))
                moveDirection -= m_Up;

            if (glm::length(moveDirection) > 0.0f)
                moveDirection = glm::normalize(moveDirection);

            // Update the camera position.
            static glm::vec2 lastMousePosition = { 0.0f, 0.0f };
            m_Position += moveDirection * velocity;
            if (m_FirstUpdate)
            {
                lastMousePosition = m_Window.GetInput().GetCursorPosition();
                m_FirstUpdate = false;
            }

            // Mouse movement
            glm::vec2 mousePosition = m_Window.GetInput().GetCursorPosition();
            float xOffset = static_cast<float>(mousePosition.x - lastMousePosition.x);
            float yOffset = static_cast<float>(lastMousePosition.y - mousePosition.y);

            //Reset cursor
            m_Window.GetInput().SetCursorPosition({ m_Window.GetSize().x / 2.0f, m_Window.GetSize().y / 2.0f });
            m_Window.GetInput().SetCursorMode(CursorMode::Disabled);

            lastMousePosition.x = static_cast<float>(m_Window.GetSize().x / 2.f);
            lastMousePosition.y = static_cast<float>(m_Window.GetSize().y / 2.f);

            xOffset *= m_MouseSensitivity;
            yOffset *= m_MouseSensitivity;

            //Set new settings
            m_Yaw += xOffset;
            m_Pitch += yOffset;

            // Cap movement
            if (m_Pitch > 89.0f)
                m_Pitch = 89.0f;
            if (m_Pitch < -89.0f)
                m_Pitch = -89.0f;
        }
        else
        {
            m_Window.GetInput().SetCursorMode(CursorMode::Shown);
            m_FirstUpdate = true;
        }

        UpdateMatrices();
    }
}

bool Camera3D::OnMouseScroll(MouseScrolledEvent& e)
{
    if (m_Window.GetInput().IsMousePressed(MouseButton::Right))
    {
        float change = m_Change * e.GetYOffset();
        m_Radius -= change;
    }

    return false;
}

void Camera3D::UpdateMatrices()
{
    glm::vec3 newFront(1.0f);
		newFront.x = glm::cos(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));
		newFront.y = glm::sin(glm::radians(m_Pitch));
		newFront.z = glm::sin(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));

		m_Front = glm::normalize(newFront);
		m_Right = glm::normalize(glm::cross(m_Front, m_Up));

		// Update everything
		m_Camera3DData.ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
		m_Camera3DData.ProjectionMatrix = glm::perspective(glm::radians(m_FOV), (float)m_Window.GetSize().x / (float)m_Window.GetSize().y, m_Near, m_Far);
		
        m_Camera3DData.ProjectionMatrix[1][1] *= -1;
}