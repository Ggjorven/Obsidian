#pragma once

#include "Obsidian/Core/Logging.hpp"
#include "Obsidian/Core/Window.hpp"

#include "Obsidian/Renderer/Device.hpp"

#include "Obsidian/Maths/Structs.hpp"

using namespace Obsidian;

////////////////////////////////////////////////////////////////////////////////////
// Camera2DData
////////////////////////////////////////////////////////////////////////////////////
struct Camera2DData
{
public:
    Maths::Mat4<float> ViewMatrix = Maths::Mat4<float>(1.0f);
    Maths::Mat4<float> ProjectionMatrix = Maths::Mat4<float>(1.0f);
};

////////////////////////////////////////////////////////////////////////////////////
// Camera2D
////////////////////////////////////////////////////////////////////////////////////
class Camera2D
{
public:
    // Constructor & Destructor
    Camera2D(const Window& window);
    ~Camera2D();

    // Methods
    void OnUpdate(float deltaTime);
    void OnEvent(Event& e);

    // Setters & getters
    void SetPosition(const Maths::Vec3<float>& position);
    inline const Maths::Vec3<float>& GetPosition() const { return m_Position; }

    void SetZoom(float zoom);
    inline float GetZoom() const { return m_Zoom; }

    // Getters
    inline const Maths::Mat4<float>& GetViewMatrix() const { return m_Camera2D.ViewMatrix; }
    inline const Maths::Mat4<float>& GetProjectionMatrix() const { return m_Camera2D.ProjectionMatrix; }
    inline const Camera2DData& GetCamera2D() const { return m_Camera2D; }

private:
    // Private methods
    void RecalculateViewMatrix();
    void RecalculateProjectionMatrix();

private:
    const Window& m_Window;
    Camera2DData m_Camera2D = {};

    Maths::Vec2<float> m_MousePosition;
    bool m_WasMouseDown = false;

    Maths::Vec3<float> m_Position = { 0.0f, 0.0f, 0.0f };
    float m_Zoom = 1.0f;
    float m_Rotation = 0.0f; // Optional: for rotating the Camera2D
};