#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/ShaderSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{
    class Device;
}

namespace Nano::Graphics::Internal
{

    class VulkanDevice;

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

}