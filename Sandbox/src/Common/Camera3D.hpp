#pragma once

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Window.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

using namespace Nano::Graphics;

////////////////////////////////////////////////////////////////////////////////////
// Camera3DData
////////////////////////////////////////////////////////////////////////////////////
struct Camera3DData
{
public:
    Maths::Mat4<float> ViewMatrix = Maths::Mat4<float>(1.0f);
    Maths::Mat4<float> ProjectionMatrix = Maths::Mat4<float>(1.0f);
};

////////////////////////////////////////////////////////////////////////////////////
// Camera3D
////////////////////////////////////////////////////////////////////////////////////
class Camera3D
{
public:
    // Constructor & Destructor
    Camera3D(const Window& window);
    ~Camera3D();

    // Methods
    void OnUpdate(float deltaTime);
    void OnEvent(Event& e);

    // Getters
    inline const Maths::Mat4<float>& GetViewMatrix() const { return m_Camera3D.ViewMatrix; }
    inline const Maths::Mat4<float>& GetProjectionMatrix() const { return m_Camera3D.ProjectionMatrix; }
    inline const Camera3DData& GetCamera3D() const { return m_Camera3D; }

private:
    // Private methods
    bool OnMouseScroll(MouseScrolledEvent& e);

private:
    const Window& m_Window;
    Camera3DData m_Camera3D;

    // Settings
    Maths::Vec3<float> m_Position = { 0.0f, 0.0f, 0.0f };
    float m_FOV = 45.0f;

    float m_Near = 0.1f;
    float m_Far = 1000.0f;
    
    float m_Yaw = 0.0f;
    float m_Pitch = 0.0f;

    Maths::Vec3<float> m_Front = { 0.0f, 0.0f, -1.0f };
    Maths::Vec3<float> m_Up = { 0.0f, 1.0f, 0.0f };
    Maths::Vec3<float> m_Right = { 1.0f, 0.0f, 0.0f };

    // ArcBall
    float m_Radius = 4.0f;
    float m_Change = 0.5f;

    float m_Speed = 0.005f;

    float m_Theta = 0.0f;
    float m_Phi = 0.0f;
    Maths::Vec2<double> m_LastPosition = { 0.0f, 0.0f };
};