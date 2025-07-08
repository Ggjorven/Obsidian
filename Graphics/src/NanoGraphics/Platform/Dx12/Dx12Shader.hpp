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

        inline D3D12_SHADER_BYTECODE GetD3D12ShaderByteCode() const { return { .pShaderBytecode = m_ByteCodeStorage.data(), .BytecodeLength = m_ByteCodeStorage.size() }; }

    private:
        const Dx12Device& m_Device;
        ShaderSpecification m_Specification;

        std::vector<uint8_t> m_ByteCodeStorage = { };
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
        std::vector<uint32_t> CompileToSPIRV(ShaderStage stage, const std::string& code, const std::string& entryPoint, ShadingLanguage language);

    private:
        shaderc::Compiler m_Compiler = {};
    };
#endif

}