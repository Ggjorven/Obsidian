#include "ngpch.h"
#include "VulkanShader.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/Shader.hpp"

namespace Nano::Graphics::Internal
{

    static_assert(std::is_same_v<Shader::Type, VulkanShader>, "Current Shader::Type is not VulkanShader and Vulkan source code is being compiled.");

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanShader::VulkanShader(const Device& device, const ShaderSpecification& specs)
        : m_Device(*reinterpret_cast<const VulkanDevice*>(&device)), m_Specification(specs)
    {
        std::span<const char> code;
        std::visit([&](auto&& arg)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::vector<char>>)
                code = const_cast<std::vector<char>&>(arg); // Note: This is worst thing I have done in my life. I can never recover.
            else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::span<const char>>)
                code = arg;
        }, specs.SPIRV);

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VK_VERIFY(vkCreateShaderModule(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &createInfo, VulkanAllocator::GetCallbacks(), &m_Shader));

        if constexpr (VulkanContext::Validation)
        {
            if (!m_Specification.DebugName.empty())
                m_Device.GetContext().SetDebugName(m_Shader, VK_OBJECT_TYPE_SHADER_MODULE, std::string(m_Specification.DebugName));
        }
    }

    VulkanShader::~VulkanShader()
    {
    }

}