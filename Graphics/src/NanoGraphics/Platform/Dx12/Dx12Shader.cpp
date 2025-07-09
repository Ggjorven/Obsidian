#include "ngpch.h"
#include "Dx12Shader.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/Shader.hpp"

#include <dxcapi.h>
#include <spirv_cross.hpp>
#include <spirv_hlsl.hpp>
#include <spirv_reflect.hpp>

#include <bit>

namespace Nano::Graphics::Internal
{

    namespace
    {

        inline constexpr static uint32_t g_ShaderModel = 67;

        ////////////////////////////////////////////////////////////////////////////////////
        // ShaderStageToShaderCMapping
        ////////////////////////////////////////////////////////////////////////////////////
        struct ShaderStageToShaderCMapping
        {
        public:
            ShaderStage Stage;

            shaderc_shader_kind ShaderCShaderKind;
        };

        ////////////////////////////////////////////////////////////////////////////////////
        // ShaderStageToShaderCMapping array
        ////////////////////////////////////////////////////////////////////////////////////
        constexpr static auto g_ShaderStageToShaderCMapping = std::to_array<ShaderStageToShaderCMapping>({
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
        // ShaderStageToDX12StageMapping
        ////////////////////////////////////////////////////////////////////////////////////
        struct ShaderStageToDX12StageMapping
        {
        public:
            ShaderStage Stage;

            std::wstring_view DX12Stage;
        };

        ////////////////////////////////////////////////////////////////////////////////////
        // ShaderStageMappingToDX12Stage array
        ////////////////////////////////////////////////////////////////////////////////////
        static constexpr auto g_ShaderStageToDX12StageMapping = std::to_array<ShaderStageToDX12StageMapping>({
            // Stage                                Dx12Stage
            { ShaderStage::None,                    L"vs_6_7" }, // Default
            { ShaderStage::Vertex,                  L"vs_6_7" },
            { ShaderStage::Fragment,                L"ps_6_7" },
            { ShaderStage::Compute,                 L"cs_6_7" },
            { ShaderStage::Geometry,                L"gs_6_7" },
            { ShaderStage::TesselationControl,      L"hs_6_7" },
            { ShaderStage::TesselationEvaluation,   L"ds_6_7" },
            { ShaderStage::Task,                    L"as_6_7" },
            { ShaderStage::Mesh,                    L"ms_6_7" },
            { ShaderStage::AllGraphics,             L"vs_6_7" }, // Default
            { ShaderStage::RayGeneration,           L"raygen_6_7" },
            { ShaderStage::AnyHit,                  L"anyhit_6_7" },
            { ShaderStage::ClosestHit,              L"closesthit_6_7" },
            { ShaderStage::Miss,                    L"miss_6_7" },
            { ShaderStage::Intersection,            L"intersection_6_7" },
            { ShaderStage::Callable,                L"callable_6_7" }
        });

        ////////////////////////////////////////////////////////////////////////////////////
        // ShaderStageToExecutionModelMapping
        ////////////////////////////////////////////////////////////////////////////////////
        struct ShaderStageToExecutionModelMapping
        {
        public:
            ShaderStage Stage;

            spv::ExecutionModel Model;
        };

        ////////////////////////////////////////////////////////////////////////////////////
        // ShaderStageToExecutionModelMapping array
        ////////////////////////////////////////////////////////////////////////////////////
        static constexpr auto g_ShaderStageToExecutionModelMapping = std::to_array<ShaderStageToExecutionModelMapping>({
            // Stage                                Model
            { ShaderStage::None,                    spv::ExecutionModelVertex }, // Default
            { ShaderStage::Vertex,                  spv::ExecutionModelVertex },
            { ShaderStage::Fragment,                spv::ExecutionModelFragment },
            { ShaderStage::Compute,                 spv::ExecutionModelGLCompute },
            { ShaderStage::Geometry,                spv::ExecutionModelGeometry },
            { ShaderStage::TesselationControl,      spv::ExecutionModelTessellationControl },
            { ShaderStage::TesselationEvaluation,   spv::ExecutionModelTessellationEvaluation },
            { ShaderStage::Task,                    spv::ExecutionModelTaskEXT },
            { ShaderStage::Mesh,                    spv::ExecutionModelMeshEXT },
            { ShaderStage::RayGeneration,           spv::ExecutionModelRayGenerationKHR },
            { ShaderStage::AnyHit,                  spv::ExecutionModelAnyHitKHR },
            { ShaderStage::ClosestHit,              spv::ExecutionModelClosestHitKHR },
            { ShaderStage::Miss,                    spv::ExecutionModelMissKHR },
            { ShaderStage::Intersection,            spv::ExecutionModelIntersectionKHR },
            { ShaderStage::Callable,                spv::ExecutionModelCallableKHR }
        });


        ////////////////////////////////////////////////////////////////////////////////////
        // Helper methods
        ////////////////////////////////////////////////////////////////////////////////////
        constexpr static shaderc_shader_kind ShaderStageToShaderCKind(ShaderStage stage)
        {
            return g_ShaderStageToShaderCMapping[(std::to_underlying(stage) ? (std::countr_zero(std::to_underlying(stage)) + 1) : 0)].ShaderCShaderKind;
        }

        constexpr static std::wstring_view ShaderStageToDX12Stage(ShaderStage stage)
        {
            return g_ShaderStageToDX12StageMapping[(std::to_underlying(stage) ? (std::countr_zero(std::to_underlying(stage)) + 1) : 0)].DX12Stage;
        }

        constexpr static spv::ExecutionModel ShaderStageToExecutionModel(ShaderStage stage)
        {
            return g_ShaderStageToExecutionModelMapping[(std::to_underlying(stage) ? (std::countr_zero(std::to_underlying(stage)) + 1) : 0)].Model;
        }

        std::vector<uint8_t> CompileHLSLToDXIL(const std::string& code, const std::wstring& entryPoint, const std::wstring& targetProfile)
        {
            // Initialization
            DxPtr<IDxcUtils> utils;
            DX_VERIFY(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));
            
            DxPtr<IDxcCompiler3> compiler;
            DX_VERIFY(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));
            
            DxPtr<IDxcIncludeHandler> includeHandler;
            DX_VERIFY(utils->CreateDefaultIncludeHandler(&includeHandler));

            // Prepare arguments
            auto arguments = std::to_array<LPCWSTR>({
                L"VirtualFileName.hlsl",        // [0] shader file name
                L"-E", entryPoint.c_str(),      // [1][2] entry point
                L"-T", targetProfile.c_str(),   // [3][4] target profile
                L"-Zi",                         // [5] debug info
                L"-Qembed_debug",               // [6] embed debug info
                L"-Zpr"                         // [7] pack matrices row-major
            });

            // Create source buffer
            DxcBuffer sourceBuffer = {
                .Ptr = code.data(),
                .Size = code.size(),
                .Encoding = DXC_CP_UTF8,
            };

            DxPtr<IDxcResult> result;
            DX_VERIFY(compiler->Compile(&sourceBuffer, arguments.data(), static_cast<UINT32>(arguments.size()), includeHandler.Get(), IID_PPV_ARGS(&result)));

            // Check for errors
            DxPtr<IDxcBlobUtf8> errors;
            result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);

            // Errors
            //if (errors && errors->GetStringLength() > 0) 
            //{
            //    output.errorMessage = std::wstring(errors->GetStringPointer(), errors->GetStringPointer() + errors->GetStringLength());
            //}

            // Output
            std::vector<uint8_t> output;

            // Get bytecode
            DxPtr<IDxcBlob> shaderBlob;
            result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);

            if (shaderBlob) 
            {
                auto* ptr = static_cast<uint8_t*>(shaderBlob->GetBufferPointer());
                size_t size = shaderBlob->GetBufferSize();
                output.assign(ptr, ptr + size);
            }

            return output;
        }

    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12Shader::Dx12Shader(const Device& device, const ShaderSpecification& specs)
        : m_Device(*api_cast<const Dx12Device*>(&device)), m_Specification(specs)
    {
        // If native was passed in, we don't need to remap SPIRV to HLSL and recompile
        if (std::holds_alternative<std::variant<std::vector<uint8_t>, std::span<const uint8_t>>>(specs.Code))
        {
            std::span<const uint8_t> code;
            std::visit([&](auto&& arg)
            {
                if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::vector<uint8_t>>)
                    code = const_cast<std::vector<uint8_t>&>(arg); // Note: This is worst thing I have done in my life. I can never recover.
                else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::span<const uint8_t>>)
                    code = arg;
                else
                    NG_UNREACHABLE();
            }, std::get<std::variant<std::vector<uint8_t>, std::span<const uint8_t>>>(m_Specification.Code));

            m_ByteCodeStorage.clear();
            m_ByteCodeStorage.insert(m_ByteCodeStorage.begin(), code.begin(), code.end());
        }
        // Remap SPIRV to HLSL and recompile
        else
        {
            std::span<const uint32_t> code;
            std::visit([&](auto&& arg)
            {
                if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::vector<uint32_t>>)
                    code = const_cast<std::vector<uint32_t>&>(arg); // Note: This is worst thing I have done in my life. I can never recover.
                else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, std::span<const uint32_t>>)
                    code = arg;
                else
                    NG_UNREACHABLE();
            }, std::get<std::variant<std::vector<uint32_t>, std::span<const uint32_t>>>(m_Specification.Code));

            // Convert to HLSL
            std::string hlslString; 
            {
                spirv_cross::CompilerHLSL compiler(code.data(), code.size()); 
                spirv_cross::CompilerHLSL::Options options;
                options.shader_model = g_ShaderModel;
            
                compiler.set_hlsl_options(options);
                compiler.set_entry_point(std::string(m_Specification.MainName), ShaderStageToExecutionModel(m_Specification.Stage));
            
                hlslString = compiler.compile();
            }

            // Compile HLSL
            {
                int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, m_Specification.MainName.data(), static_cast<int>(m_Specification.MainName.size()), nullptr, 0);

                NG_ASSERT((length != 0), "[Dx12Context] Failed to convert std::string to std::wstring. Error: {0}", GetLastError());

                std::wstring entryName(length, L'\0');
                (void)MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, m_Specification.MainName.data(), static_cast<int>(m_Specification.MainName.size()), entryName.data(), length);

                m_ByteCodeStorage = CompileHLSLToDXIL(hlslString, entryName, std::wstring(ShaderStageToDX12Stage(m_Specification.Stage)));
            }
        }
    }

    Dx12Shader::~Dx12Shader()
    {
    }

    // Note: Copied from VulkanShaderCompiler.hpp
    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor 
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12ShaderCompiler::Dx12ShaderCompiler()
    {
    }

    Dx12ShaderCompiler::~Dx12ShaderCompiler()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    std::vector<uint32_t> Dx12ShaderCompiler::CompileToSPIRV(ShaderStage stage, const std::string& code, const std::string& entryPoint, ShadingLanguage language)
    {
        NG_ASSERT((!code.empty()), "[Dx12ShaderCompiler] Empty string passed in as shader code.");

        // Set language
        shaderc::CompileOptions options = {};
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);

        // Note: Forces the shaders to manually set all registers/sets/bindings
        options.SetAutoBindUniforms(false);
        
        if (language == ShadingLanguage::GLSL)
            options.SetSourceLanguage(shaderc_source_language_glsl);
        else if (language == ShadingLanguage::HLSL)
        {
            options.SetSourceLanguage(shaderc_source_language_hlsl);
            options.SetTargetSpirv(shaderc_spirv_version_1_6);

            options.AddMacroDefinition("HLSL");
            options.SetHlslIoMapping(true); // Note: Needed for `register(b0, space0)` layout
        }

        shaderc::SpvCompilationResult module = m_Compiler.CompileGlslToSpv(code, ShaderStageToShaderCKind(stage), "", entryPoint.c_str(), options);

        NG_ASSERT((module.GetCompilationStatus() == shaderc_compilation_status_success), "[Dx12ShaderCompiler] Error compiling shader: {0}", module.GetErrorMessage());

        return std::vector<uint32_t>(module.cbegin(), module.cend());
    }

    std::vector<uint8_t> Dx12ShaderCompiler::CompileToNative(ShaderStage stage, const std::string& code, const std::string& entryPoint, ShadingLanguage language)
    {
        NG_ASSERT((!code.empty()), "[Dx12ShaderCompiler] Empty string passed in as shader code.");

        // Get the entrypoint as a wstring
        int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, entryPoint.data(), static_cast<int>(entryPoint.size()), nullptr, 0);

        NG_ASSERT((length != 0), "[Dx12Context] Failed to convert std::string to std::wstring. Error: {0}", GetLastError());

        std::wstring entryName(length, L'\0');
        (void)MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, entryPoint.data(), static_cast<int>(entryPoint.size()), entryName.data(), length);

        // Note: We can directy compile HLSL to DXIL
        if (language == ShadingLanguage::HLSL)
            return CompileHLSLToDXIL(code, entryName, std::wstring(ShaderStageToDX12Stage(stage)));

        std::vector<uint32_t> spirv = CompileToSPIRV(stage, code, entryPoint, language);

        // Convert the GLSL to HLSL
        std::string hlslString;
        {
            spirv_cross::CompilerHLSL compiler(spirv.data(), spirv.size());
            spirv_cross::CompilerHLSL::Options options;
            options.shader_model = g_ShaderModel;

            compiler.set_hlsl_options(options);
            compiler.set_entry_point(entryPoint, ShaderStageToExecutionModel(stage));

            hlslString = compiler.compile();
        }

        return CompileHLSLToDXIL(code, entryName, std::wstring(ShaderStageToDX12Stage(stage)));
    }

}