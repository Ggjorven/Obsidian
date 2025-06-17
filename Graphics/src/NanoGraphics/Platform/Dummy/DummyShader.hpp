#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/ShaderSpec.hpp"

#include <Nano/Nano.hpp>

#include <vector>

namespace Nano::Graphics
{
    class Device;
}

namespace Nano::Graphics::Internal
{

    class DummyShader;
    class DummyShaderCompiler;

#if 1 //defined(NG_API_DUMMY)
    ////////////////////////////////////////////////////////////////////////////////////
    // DummyShader
    ////////////////////////////////////////////////////////////////////////////////////
    class DummyShader
    {
    public:
        // Constructors & Destructor
        inline constexpr DummyShader(const Device& device, const ShaderSpecification& specs)
            : m_Specification(specs) { (void)device; }
        constexpr ~DummyShader() = default;

        // Getters
        inline constexpr const ShaderSpecification& GetSpecification() const { return m_Specification; }

    private:
        ShaderSpecification m_Specification;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // DummyShaderCompiler
    ////////////////////////////////////////////////////////////////////////////////////
    class DummyShaderCompiler
    {
    public:
        // Constructors & Destructor
        inline constexpr DummyShaderCompiler() = default;
        constexpr ~DummyShaderCompiler() = default;

        // Methods
        inline std::vector<char> CompileToSPIRV(ShaderStage stage, const std::string& code, ShadingLanguage language) { (void)stage; (void)code; (void)language; return {}; }
    };
#endif

}