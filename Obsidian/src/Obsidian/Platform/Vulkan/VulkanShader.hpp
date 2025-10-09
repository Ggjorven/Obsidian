#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/API.hpp"
#include "Obsidian/Renderer/ShaderSpec.hpp"

#include "Obsidian/Platform/Vulkan/Vulkan.hpp"

#include <Nano/Nano.hpp>

#include <shaderc/shaderc.hpp>

#include <vector>

namespace Obsidian
{
    class Device;
}

namespace Obsidian::Internal
{

    class VulkanDevice;
    class VulkanShader;
    class VulkanShaderCompiler;

#if defined(OB_API_VULKAN)
    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanShader
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanShader
    {
    public:
        // Constructors & Destructor
        VulkanShader(const Device& device, const ShaderSpecification& specs);
        ~VulkanShader();

        // Getters
        inline const ShaderSpecification& GetSpecification() const { return m_Specification; }

        // Internal getters
        inline VkShaderModule GetVkShaderModule() const { return m_Shader; }

        inline const VulkanDevice& GetVulkanDevice() const { return m_Device; }

    private:
        const VulkanDevice& m_Device;
        ShaderSpecification m_Specification;

        VkShaderModule m_Shader = VK_NULL_HANDLE;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanShaderCompiler
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanShaderCompiler
    {
    public:
        // Constructors & Destructor
        VulkanShaderCompiler();
        ~VulkanShaderCompiler();

        // Methods
        std::vector<uint32_t> CompileToSPIRV(ShaderStage stage, const std::string& code, const std::string& entryPoint, ShadingLanguage language);

        std::vector<uint32_t> CompileToNative(ShaderStage stage, const std::string& code, const std::string& entryPoint, ShadingLanguage language);

    private:
        shaderc::Compiler m_Compiler = {};
    };
#endif

}