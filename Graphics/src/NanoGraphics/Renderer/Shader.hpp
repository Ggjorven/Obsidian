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
        inline std::vector<uint32_t> CompileToSPIRV(ShaderStage stage, const std::string& code, const std::string& entryPoint = "main", ShadingLanguage language = ShadingLanguage::GLSL) { return m_Impl->CompileToSPIRV(stage, code, entryPoint, language); }

    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class APICaster;
    };

}