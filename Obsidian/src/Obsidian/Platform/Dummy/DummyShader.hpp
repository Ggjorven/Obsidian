#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/ShaderSpec.hpp"

#include <Nano/Nano.hpp>

#include <vector>

namespace Obsidian
{
    class Device;
}

namespace Obsidian::Internal
{

    class DummyShader;
    class DummyShaderCompiler;

#if 1 //defined(OB_API_DUMMY)
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
        inline std::vector<uint32_t> CompileToSPIRV(ShaderStage stage, const std::string& code, const std::string& entryPoint, ShadingLanguage language) { (void)stage; (void)code; (void)entryPoint; (void)language; return {}; }
    };
#endif

}