#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/ShaderSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanShader.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Shader.hpp"
#include "NanoGraphics/Platform/Dummy/DummyShader.hpp"

#include <Nano/Nano.hpp>

#include <vector>

namespace Nano::Graphics
{

    class Device;

    ////////////////////////////////////////////////////////////////////////////////////
    // Shader
    ////////////////////////////////////////////////////////////////////////////////////
    class Shader
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanShader>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12Shader>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyShader>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyShader>
        >;
    public:
        // Destructor
        ~Shader() = default;

        // Getters
        inline const ShaderSpecification& GetSpecification() const { return m_Impl->GetSpecification(); }

    public: //private:
        // Constructor
        inline Shader(const Device& device, const ShaderSpecification& specs) { m_Impl.Construct(device, specs); }

    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class Device;
        friend class APICaster;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ShaderCompiler
    ////////////////////////////////////////////////////////////////////////////////////
    class ShaderCompiler
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanShaderCompiler>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12ShaderCompiler>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyShaderCompiler>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyShaderCompiler>
        >;
    public:
        // Constructor & Destructor
        inline ShaderCompiler() { m_Impl.Construct(); }
        ~ShaderCompiler() = default;

        // Methods
        // Note: SPIRV is the format that the underlying API can use for all backends.
        inline std::vector<uint32_t> CompileToSPIRV(ShaderStage stage, const std::string& code, const std::string& entryPoint = "main", ShadingLanguage language = ShadingLanguage::GLSL) { return m_Impl->CompileToSPIRV(stage, code, entryPoint, language); }

#if defined(NG_API_VULKAN)
        // Note: This API should not be use unless you explicitly know what you are doing.
        // This can slightly improve performance for a runtime application which is no longer in development.
        inline std::vector<uint32_t> CompileToNative(ShaderStage stage, const std::string& code, const std::string& entryPoint = "main", ShadingLanguage language = ShadingLanguage::GLSL) { return m_Impl->CompileToNative(stage, code, entryPoint, language); }
#elif defined(NG_API_DX12)
        // Note: This API should not be use unless you explicitly know what you are doing.
        // This can slightly improve performance for a runtime application which is no longer in development.
        inline std::vector<uint8_t> CompileToNative(ShaderStage stage, const std::string& code, const std::string& entryPoint = "main", ShadingLanguage language = ShadingLanguage::GLSL) { return m_Impl->CompileToNative(stage, code, entryPoint, language); }
#elif defined(NG_API_DUMMY)
        // Note: This API should not be use unless you explicitly know what you are doing.
        // This can slightly improve performance for a runtime application which is no longer in development.
        // Note 2: Dummy doesn't have a special Native function so it return the SPIRV.
        inline std::vector<uint32_t> CompileToNative(ShaderStage stage, const std::string& code, const std::string& entryPoint = "main", ShadingLanguage language = ShadingLanguage::GLSL) { return m_Impl->CompileToSPIRV(stage, code, entryPoint, language); }
#endif
    
    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class APICaster;
    };

}