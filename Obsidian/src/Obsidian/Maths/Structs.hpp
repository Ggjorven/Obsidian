#pragma once

#include <cstdint>

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace Obsidian::Internal::Maths
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Vec2
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    struct Vec2;

    template<> struct Vec2<bool>            { using Type = glm::bvec2; };
    template<> struct Vec2<int>             { using Type = glm::ivec2; };
    template<> struct Vec2<unsigned int>    { using Type = glm::uvec2; };
    template<> struct Vec2<float>           { using Type = glm::vec2; };
    template<> struct Vec2<double>          { using Type = glm::dvec2; };

    ////////////////////////////////////////////////////////////////////////////////////
    // Vec3
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    struct Vec3;

    template<> struct Vec3<bool>            { using Type = glm::bvec3; };
    template<> struct Vec3<int>             { using Type = glm::ivec3; };
    template<> struct Vec3<unsigned int>    { using Type = glm::uvec3; };
    template<> struct Vec3<float>           { using Type = glm::vec3; };
    template<> struct Vec3<double>          { using Type = glm::dvec3; };

    ////////////////////////////////////////////////////////////////////////////////////
    // Vec4
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    struct Vec4;

    template<> struct Vec4<bool>            { using Type = glm::bvec4; };
    template<> struct Vec4<int>             { using Type = glm::ivec4; };
    template<> struct Vec4<unsigned int>    { using Type = glm::uvec4; };
    template<> struct Vec4<float>           { using Type = glm::vec4; };
    template<> struct Vec4<double>          { using Type = glm::dvec4; };

    ////////////////////////////////////////////////////////////////////////////////////
    // Mat3
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    struct Mat3;

    template<> struct Mat3<float>           { using Type = glm::mat3; };
    template<> struct Mat3<double>          { using Type = glm::dmat3; };

    ////////////////////////////////////////////////////////////////////////////////////
    // Mat4
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    struct Mat4;

    template<> struct Mat4<int>             { using Type = glm::imat4x4; };
    template<> struct Mat4<unsigned int>    { using Type = glm::umat4x4; };
    template<> struct Mat4<float>           { using Type = glm::mat4; };
    template<> struct Mat4<double>          { using Type = glm::dmat4; };

} 

namespace Obsidian::Maths
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Structs
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename T = float>
    using Vec2 = typename Internal::Maths::Vec2<T>::Type;
    template<typename T = float>
    using Vec3 = typename Internal::Maths::Vec3<T>::Type;
    template<typename T = float>
    using Vec4 = typename Internal::Maths::Vec4<T>::Type;

    template<typename T = float>
    using Mat3 = typename Internal::Maths::Mat3<T>::Type;
    template<typename T = float>
    using Mat4 = typename Internal::Maths::Mat4<T>::Type;

}