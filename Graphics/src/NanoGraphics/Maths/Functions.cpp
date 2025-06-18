#include "ngpch.h"
#include "Functions.hpp"

#include "NanoGraphics/Core/Logging.hpp"

namespace Nano::Graphics::Maths
{

	////////////////////////////////////////////////////////////////////////////////////
	// Vectors
	////////////////////////////////////////////////////////////////////////////////////
	Vec3<float> Normalize(const Vec3<float>& vector)
	{
		return glm::normalize(vector);
	}

	Vec3<float> Cross(const Vec3<float>& a, const Vec3<float>& b)
	{
		return glm::cross(a, b);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Matrices
	////////////////////////////////////////////////////////////////////////////////////
	Mat4<float> Perspective(float fov, float aspectRatio, float nearClip, float farClip)
	{
		return glm::perspective(fov, aspectRatio, nearClip, farClip);
	}

	Mat4<float> Orthographic(float aspectRatio, float zoom)
	{
		return Orthographic(-aspectRatio * zoom, aspectRatio * zoom, -1.0f * zoom, 1.0f * zoom, -1.0f, 1.0f);
	}

	Mat4<float> Orthographic(float left, float right, float bottom, float top, float nearClip, float farClip)
	{
		return glm::ortho(left, right, bottom, top, nearClip, farClip);
	}

	Mat4<float> LookAt(const Vec3<float>& position, const Vec3<float>& target, const Vec3<float>& up)
	{
		return glm::lookAt(position, target, up);
	}

	Mat4<float> Translate(const Mat4<float>& matrix, const Vec3<float>& translation)
	{
		return glm::translate(matrix, translation);
	}

	Mat4<float> Rotate(const Mat4<float>& matrix, float angle, const Vec3<float>& axis)
	{
		return glm::rotate(matrix, angle, axis);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Trigonometry
	////////////////////////////////////////////////////////////////////////////////////
	float Sin(float angle)
	{
		return glm::sin(angle);
	}

	float Cos(float angle)
	{
		return glm::cos(angle);
	}

	float Tan(float angle)
	{
		return glm::tan(angle);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Utils
	///////////////////////////////////////////////////////////////////////////////////
	float Radians(float degrees)
	{
		return glm::radians(degrees);
	}

	float AspectRatio(uint32_t width, uint32_t height)
	{
		return static_cast<float>(width) / static_cast<float>(height);
	}

}