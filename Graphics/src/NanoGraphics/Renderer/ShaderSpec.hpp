#pragma once

#include <Nano/Nano.hpp>

#include <cstdint>
#include <span>
#include <vector>
#include <variant>

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Flags
    ////////////////////////////////////////////////////////////////////////////////////
    enum class ShaderStage : uint16_t
    {
        None = 0,

        Vertex = 1 << 0,
        Fragment = 1 << 1,
        Pixel = Fragment,
        Compute = 1 << 2,
        Geometry = 1 << 3,
        TesselationControl = 1 << 4,
        Hull = TesselationControl,
        TesselationEvaluation = 1 << 5,
        Domain = TesselationEvaluation,
        Task = 1 << 6,
        Amplification = Task,
        Mesh = 1 << 7,
        AllGraphics = 1 << 8,

        RayGeneration = 1 << 9,
        AnyHit = 1 << 10,
        ClosestHit = 1 << 11,
        Miss = 1 << 12,
        Intersection = 1 << 13,
        Callable = 1 << 14,
        //AllRayTracing = 1 << 15,
    };

    NANO_DEFINE_BITWISE(ShaderStage)

    ////////////////////////////////////////////////////////////////////////////////////
    // ShaderSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct ShaderSpecification
    {
    public:
        ShaderStage Stage = ShaderStage::None;
        std::variant<std::vector<char>, std::span<const char>> SPIRV = {};
        
        std::string_view DebugName = {};

    public:
        // Setters
        inline ShaderSpecification& SetSPIRV(std::vector<char>&& ownedSPIRV) { SPIRV = std::move(ownedSPIRV); return *this; }
        inline ShaderSpecification& SetSPIRV(const std::vector<char>& ownedSPIRV) { SPIRV = ownedSPIRV; return *this; }
        inline constexpr ShaderSpecification& SetSPIRV(std::span<const char> viewedSPIRV) { SPIRV = viewedSPIRV; return *this; }
        inline constexpr ShaderSpecification& SetDebugName(std::string_view name) { DebugName = name; return *this; }
    };

}