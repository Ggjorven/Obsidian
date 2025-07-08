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
            { ShaderStage::Callable,                shaderc_glsl_callable_shader }
        });

        ////////////////////////////////////////////////////////////////////////////////////
        // Helper methods
        ////////////////////////////////////////////////////////////////////////////////////
        constexpr static shaderc_shader_kind ShaderStageToShaderCKind(ShaderStage stage)
        {
            return g_ShaderStageMapping[(std::to_underlying(stage) ? (std::countr_zero(std::to_underlying(stage)) + 1) : 0)].ShaderCShaderKind;
        }

    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanShader::VulkanShader(const Device& device, const ShaderSpecification& specs)
        : m_Device(*api_cast<const VulkanDevice*>(&device)), m_Specification(specs)
    {
        std::span<const uint32_t> code;
        std::visit([&](auto&& arg)
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::vector<uint32_t>>)
                code = const_cast<std::vector<uint32_t>&>(arg); // Note: This is worst thing I have done in my life. I can never recover.
            else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::span<const uint32_t>>)
                code = arg;
        }, specs.SPIRV);

        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size() * sizeof(uint32_t);
        createInfo.pCode = code.data();

        VK_VERIFY(vkCreateShaderModule(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &createInfo, VulkanAllocator::GetCallbacks(), &m_Shader));

        if constexpr (Information::Validation)
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
    }

    VulkanShaderCompiler::~VulkanShaderCompiler()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    std::vector<uint32_t> VulkanShaderCompiler::CompileToSPIRV(ShaderStage stage, const std::string& code, const std::string& entryPoint, ShadingLanguage language)
    {
        NG_ASSERT((!code.empty()), "[VkShaderCompiler] Empty string passed in as shader code.");

        // Set language
        shaderc::CompileOptions options = {};
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        
        if (language == ShadingLanguage::GLSL)      
            options.SetSourceLanguage(shaderc_source_language_glsl);
        else if (language == ShadingLanguage::HLSL)
        {
            options.SetSourceLanguage(shaderc_source_language_hlsl);
            options.SetTargetSpirv(shaderc_spirv_version_1_6);

            options.AddMacroDefinition("HLSL");
            options.SetAutoBindUniforms(false);
            options.SetHlslIoMapping(true); // Note: Needed for `register(b0, space0)` layout
        }

        shaderc::SpvCompilationResult module = m_Compiler.CompileGlslToSpv(code, ShaderStageToShaderCKind(stage), "", entryPoint.c_str(), options);

        NG_ASSERT((module.GetCompilationStatus() == shaderc_compilation_status_success), "[VkShaderCompiler] Error compiling shader: {0}", module.GetErrorMessage());

        return std::vector<uint32_t>(module.cbegin(), module.cend());
    }

}