#pragma once

#include <cstdint>
#include <type_traits>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "NanoGraphics/Maths/Structs.hpp"

namespace Nano::Graphics::Maths
{

	////////////////////////////////////////////////////////////////////////////////////
	// Vectors
	////////////////////////////////////////////////////////////////////////////////////
	Vec3<float> Normalize(const Vec3<float>& vector);
	Vec3<float> Cross(const Vec3<float>& a, const Vec3<float>& b);

	////////////////////////////////////////////////////////////////////////////////////
	// Matrices
	////////////////////////////////////////////////////////////////////////////////////
	Mat4<float> Perspective(float fov, float aspectRatio, float nearClip = 0.1f, float farClip = 100.0f); // Note: fov is in radians

	Mat4<float> Orthographic(float aspectRatio, float zoom = 1.0f);
	Mat4<float> Orthographic(float left, float right, float bottom, float top, float nearClip = -1.0f, float farClip = 1.0f);

	Mat4<float> LookAt(const Vec3<float>& position, const Vec3<float>& target, const Vec3<float>& up = { 0.0f, 1.0f, 0.0f });

	Mat4<float> Translate(const Mat4<float>& matrix, const Vec3<float>& translation);

	Mat4<float> Rotate(const Mat4<float>& matrix, float angle, const Vec3<float>& axis); // Note: Angle in radians

	Mat4<float> Inverse(const Mat4<float>& matrix);

	////////////////////////////////////////////////////////////////////////////////////
	// Trigonometry 
	////////////////////////////////////////////////////////////////////////////////////
	float Sin(float angle); // Note: Angle in radians
	float Cos(float angle); // Note: Angle in radians
	float Tan(float angle); // Note: Angle in radians

	////////////////////////////////////////////////////////////////////////////////////
	// Utils
	////////////////////////////////////////////////////////////////////////////////////
	float Radians(float degrees);

	float Clamp(float value, float min, float max);

	float AspectRatio(uint32_t width, uint32_t height);

	// Note: The underlying maths library is glm, glm works in the opengl coordinate space
	// we support multiple API's and not every API has the same projection space, so we have
	// one general function to correct the projection matrix to the current API's space
	// after all calculations have been done to it.
	Mat4<float> ApplyProjectionCorrection(const Mat4<float>& matrix);

	////////////////////////////////////////////////////////////////////////////////////
	// Values
	////////////////////////////////////////////////////////////////////////////////////
	template<typename T> 
	T Pi() requires(std::is_floating_point_v<T>)
	{
		return glm::pi<T>();
	}

	template<typename T>
	T HalfPi() requires(std::is_floating_point_v<T>)
	{
		return glm::half_pi<T>();
	}

}