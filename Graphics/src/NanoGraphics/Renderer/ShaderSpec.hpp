#pragma once

#include <Nano/Nano.hpp>

#include <cstdint>
#include <span>
#include <vector>
#include <variant>
#include <string>

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Flags
    ////////////////////////////////////////////////////////////////////////////////////
    enum class ShadingLanguage : uint8_t
    {
        GLSL = 0,
        HLSL
    };

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
        std::string_view MainName = "main";

        // Note: Only use the native type if you know what you are doing
        std::variant<
#if defined(NG_API_VULKAN)
            // Note: On vulkan we can directly use the SPIRV variant
#elif defined(NG_API_DX12)
            // Note: On dx12 we use DXIL
            std::variant<std::vector<uint8_t>, std::span<const uint8_t>>,
#elif defined(NG_API_DUMMY)
            // Note: On dummy we directly use the SPIRV variant
#endif
            std::variant<std::vector<uint32_t>, std::span<const uint32_t>>
        > Code = {};

        std::string DebugName = {};

    public:
        // Setters
        inline constexpr ShaderSpecification& SetShaderStage(ShaderStage stage) { Stage = stage; return *this; }
        inline constexpr ShaderSpecification& SetMainName(std::string_view name) { MainName = name; return *this; }

        inline ShaderSpecification& SetSPIRV(std::vector<uint32_t>&& ownedSPIRV) { Code = std::move(ownedSPIRV); return *this; }
        inline constexpr ShaderSpecification& SetSPIRV(std::span<const uint32_t> viewedSPIRV) { Code = viewedSPIRV; return *this; }

#if defined(NG_API_VULKAN)
        // Note: Only use the native type if you know what you are doing
        inline ShaderSpecification& SetNative(std::vector<uint32_t>&& ownedSPIRV) { Code = std::move(ownedSPIRV); return *this; }
        inline constexpr ShaderSpecification& SetNative(std::span<const uint32_t> viewedSPIRV) { Code = viewedSPIRV; return *this; }
#elif defined(NG_API_DX12)
        // Note: Only use the native type if you know what you are doing
        inline ShaderSpecification& SetNative(std::vector<uint8_t>&& ownedDXIL) { Code = std::move(ownedDXIL); return *this; }
        inline constexpr ShaderSpecification& SetNative(std::span<const uint8_t> viewDXIL) { Code = viewDXIL; return *this; }
#elif defined(NG_API_DUMMY)
        // Note: Only use the native type if you know what you are doing
        // Note 2: On dummy we directly use the SPIRV variant
        inline ShaderSpecification& SetNative(std::vector<uint32_t>&& ownedSPIRV) { Code = std::move(ownedSPIRV); return *this; }
        inline constexpr ShaderSpecification& SetNative(std::span<const uint32_t> viewedSPIRV) { Code = viewedSPIRV; return *this; }
#endif

        inline ShaderSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }
    };

}