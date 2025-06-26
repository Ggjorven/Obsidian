#pragma once

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/ShaderSpec.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"

#include <Nano/Nano.hpp>

#include <shaderc/shaderc.hpp>

namespace Nano::Graphics
{
    class Device;
}

namespace Nano::Graphics::Internal
{

    class Dx12Device;
    class Dx12Shader;
    class Dx12ShaderCompiler;

#if defined(NG_API_DX12)
    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12Shader
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12Shader
    {
    public:
        // Constructors & Destructor
        Dx12Shader(const Device& device, const ShaderSpecification& specs);
        ~Dx12Shader();

        // Getters
        inline const ShaderSpecification& GetSpecification() const { return m_Specification; }

        // Internal getters
        inline const Dx12Device& GetDx12Device() const { return m_Device; }

    private:
        const Dx12Device& m_Device;
        ShaderSpecification m_Specification;

        D3D12_SHADER_BYTECODE m_ByteCode = {};
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12ShaderCompiler
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12ShaderCompiler
    {
    public:
        // Constructors & Destructor
        Dx12ShaderCompiler();
        ~Dx12ShaderCompiler();

        // Methods
        std::vector<char> CompileToSPIRV(ShaderStage stage, const std::string& code, ShadingLanguage language);

    private:
        shaderc::Compiler m_Compiler = {};
    };
#endif

}