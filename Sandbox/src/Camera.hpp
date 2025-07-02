#pragma once

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Window.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

using namespace Nano::Graphics;

struct alignas(256) CameraData
{
public:
    Maths::Mat4<float> ViewMatrix = Maths::Mat4<float>(1.0f);
    Maths::Mat4<float> ProjectionMatrix = Maths::Mat4<float>(1.0f);
};

class Camera
{
public:
    Camera(const Window& window);
    ~Camera();

    void OnUpdate(float deltaTime);
    void OnEvent(Event& e);

    // Setters & getters
    void SetPosition(const Maths::Vec3<float>& position);
    inline const Maths::Vec3<float>& GetPosition() const { return m_Position; }

    void SetZoom(float zoom);
    inline float GetZoom() const { return m_Zoom; }

    inline const Maths::Mat4<float>& GetViewMatrix() const { return m_Camera.ViewMatrix; }
    inline const Maths::Mat4<float>& GetProjectionMatrix() const { return m_Camera.ProjectionMatrix; }
    inline const CameraData& GetCamera() const { return m_Camera; }

private:
    void RecalculateViewMatrix();
    void RecalculateProjectionMatrix();

private:
    const Window& m_Window;
    CameraData m_Camera = {};

    Maths::Vec2<float> m_MousePosition;
    bool m_WasMouseDown = false;

    Maths::Vec3<float> m_Position = { 0.0f, 0.0f, 0.0f };
    float m_Zoom = 1.0f;
    float m_Rotation = 0.0f; // Optional: for rotating the camera
};