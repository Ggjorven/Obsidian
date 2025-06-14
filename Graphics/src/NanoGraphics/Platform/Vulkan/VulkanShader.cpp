#include "ngpch.h"
#include "VulkanShader.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/Shader.hpp"

#include <bit>

namespace Nano::Graphics::Internal
{

    namespace
    {

        ////////////////////////////////////////////////////////////////////////////////////
        // ShaderStageMapping
        ////////////////////////////////////////////////////////////////////////////////////
        struct ShaderStageMapping
        {
        public:
            ShaderStage Stage;

            shaderc_shader_kind ShaderCShaderKind;
        };

        ////////////////////////////////////////////////////////////////////////////////////
        // ShaderStageMapping array
        ////////////////////////////////////////////////////////////////////////////////////
        constexpr static auto g_ShaderStageMapping = std::to_array<ShaderStageMapping>({
            // Stage                                ShaderCShaderKind
            { ShaderStage::None,                    shaderc_glsl_vertex_shader }, // Default
            { ShaderStage::Vertex,                  shaderc_glsl_vertex_shader },
            { ShaderStage::Fragment,                shaderc_glsl_fragment_shader },
            { ShaderStage::Compute,                 shaderc_glsl_compute_shader },
            { ShaderStage::Geometry,                shaderc_glsl_geometry_shader },
            { ShaderStage::TesselationControl,      shaderc_glsl_tess_control_shader },
            { ShaderStage::TesselationEvaluation,   shaderc_glsl_tess_evaluation_shader },
            { ShaderStage::Task,                    shaderc_glsl_task_shader },
            { ShaderStage::Mesh,                    shaderc_glsl_mesh_shader },
            { ShaderStage::AllGraphics,             shaderc_glsl_vertex_shader }, // Default
            { ShaderStage::RayGeneration,           shaderc_glsl_raygen_shader },
            { ShaderStage::AnyHit,                  shaderc_glsl_anyhit_shader },
            { ShaderStage::ClosestHit,              shaderc_glsl_closesthit_shader },
            { ShaderStage::Miss,                    shaderc_glsl_miss_shader },
            { ShaderStage::Intersection,            shaderc_glsl_intersection_shader },
            { ShaderStage::Callable,                shaderc_glsl_callable_shader },
        });

        ////////////////////////////////////////////////////////////////////////////////////
        // Helper methods
        ////////////////////////////////////////////////////////////////////////////////////
        static shaderc_shader_kind ShaderStageToShaderCKind(ShaderStage stage)
        {
            return g_ShaderStageMapping[(std::to_underlying(stage) ? (std::countr_zero(std::to_underlying(stage)) + 1) : 0)].ShaderCShaderKind;
        }

    }

    static_assert(std::is_same_v<Shader::Type, VulkanShader>, "Current Shader::Type is not VulkanShader and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<ShaderCompiler::Type, VulkanShaderCompiler>, "Current ShaderCompiler::Type is not VulkanShaderCompiler and Vulkan source code is being compiled.");

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

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanShaderCompiler::VulkanShaderCompiler()
    {
        #if defined(NG_PLATFORM_MACOS)
            m_CompileOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        #else
            m_CompileOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
        #endif
    }

    VulkanShaderCompiler::~VulkanShaderCompiler()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    std::vector<char> VulkanShaderCompiler::CompileToSPIRV(ShaderStage stage, const std::string& code, ShadingLanguage language)
    {
        NG_ASSERT((!code.empty()), "[VkShaderCompiler] Empty string passed in as shader code.");

        // Set language
        if (language == ShadingLanguage::GLSL)      m_CompileOptions.SetSourceLanguage(shaderc_source_language_glsl);
        else if (language == ShadingLanguage::HLSL) m_CompileOptions.SetSourceLanguage(shaderc_source_language_hlsl);

        shaderc::SpvCompilationResult module = m_Compiler.CompileGlslToSpv(code, ShaderStageToShaderCKind(stage), "", m_CompileOptions);

        NG_ASSERT((module.GetCompilationStatus() == shaderc_compilation_status_success), "[VkShaderCompiler] Error compiling shader: {0}", module.GetErrorMessage());

        // Convert SPIR-V code to vector<char>
        const uint32_t* data = module.cbegin();
        const size_t numWords = module.cend() - module.cbegin();
        const size_t sizeInBytes = numWords * sizeof(uint32_t);
        const char* bytes = reinterpret_cast<const char*>(data);

        return std::vector<char>(bytes, bytes + sizeInBytes);
    }

}