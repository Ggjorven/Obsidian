#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/ShaderSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanShader.hpp"
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
            Types::EnumToType<Information::Structs::RenderingAPI::D3D12, Internal::DummyShader>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyShader>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyShader>
        >;
    public:
        // Destructor
        ~Shader() = default;

        // Getters
        inline const ShaderSpecification& GetSpecification() const { return m_Shader->GetSpecification(); }

    public: //private:
        // Constructor
        inline Shader(const Device& device, const ShaderSpecification& specs) { m_Shader.Construct(device, specs); }

    private:
        // Helper getter
        inline Type& APICasterGet() { return m_Shader.Get(); }

    private:
        Internal::APIObject<Type> m_Shader = {};

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
            Types::EnumToType<Information::Structs::RenderingAPI::D3D12, Internal::DummyShaderCompiler>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyShaderCompiler>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyShaderCompiler>
        >;
    public:
        // Constructor & Destructor
        inline ShaderCompiler() { m_ShaderCompiler.Construct(); }
        ~ShaderCompiler() = default;

        // Methods
        inline std::vector<char> CompileToSPIRV(ShaderStage stage, const std::string& code, ShadingLanguage language = ShadingLanguage::GLSL) { return m_ShaderCompiler->CompileToSPIRV(stage, code, language); }

    private:
        // Helper getter
        inline Type& APICasterGet() { return m_ShaderCompiler.Get(); }

    private:
        Internal::APIObject<Type> m_ShaderCompiler = {};

        friend class APICaster;
    };

}