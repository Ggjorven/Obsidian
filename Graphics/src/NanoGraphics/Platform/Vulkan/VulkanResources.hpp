#pragma once

#include "NanoGraphics/Core/Core.hpp"
#include "NanoGraphics/Core/Logging.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/BindingsSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"
#include "NanoGraphics/Renderer/BufferSpec.hpp"
#include "NanoGraphics/Renderer/SwapchainSpec.hpp"
#include "NanoGraphics/Renderer/CommandListSpec.hpp"
#include "NanoGraphics/Renderer/FramebufferSpec.hpp"
#include "NanoGraphics/Renderer/RenderpassSpec.hpp"
#include "NanoGraphics/Renderer/ShaderSpec.hpp"
#include "NanoGraphics/Renderer/PipelineSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <cstdint>
#include <bit>
#include <type_traits>

namespace Nano::Graphics::Internal
{

#if defined(NG_API_VULKAN)
    ////////////////////////////////////////////////////////////////////////////////////
    // ResourceStateMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct ResourceStateMapping
    {
    public:
        ResourceState State;

        VkPipelineStageFlags2 StageFlags;
        VkAccessFlags2 AccessMask;
        VkImageLayout ImageLayout;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ResourceStateMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_ResourceStateMapping = std::to_array<ResourceStateMapping>({
        // State                            StageFlags                                          AccessMask                                      ImageLayout
        { ResourceState::Unknown,           VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,                VK_ACCESS_2_NONE,                               VK_IMAGE_LAYOUT_UNDEFINED },
        { ResourceState::Common,            VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,                VK_ACCESS_2_NONE,                               VK_IMAGE_LAYOUT_UNDEFINED },
        { ResourceState::StorageBuffer,     VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,               VK_ACCESS_2_UNIFORM_READ_BIT,                   VK_IMAGE_LAYOUT_UNDEFINED },
        { ResourceState::VertexBuffer,      VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT,               VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT,          VK_IMAGE_LAYOUT_UNDEFINED },
        { ResourceState::IndexBuffer,       VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT,               VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT_KHR,      VK_IMAGE_LAYOUT_UNDEFINED },
        { ResourceState::IndirectArgument,  VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT,              VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT,          VK_IMAGE_LAYOUT_UNDEFINED },
        { ResourceState::ShaderResource,    VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,               VK_ACCESS_2_SHADER_READ_BIT,                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
        { ResourceState::UnorderedAccess,   VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,               VK_ACCESS_2_SHADER_READ_BIT | 
                                                                                                VK_ACCESS_2_SHADER_WRITE_BIT,                   VK_IMAGE_LAYOUT_GENERAL },
        { ResourceState::RenderTarget,      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,    VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | 
                                                                                                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
        { ResourceState::DepthWrite,        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | 
                                            VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | 
                                                                                                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL },
        { ResourceState::DepthRead,         VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | 
                                            VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,  VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL },
        { ResourceState::StreamOut,         VK_PIPELINE_STAGE_2_TRANSFORM_FEEDBACK_BIT_EXT,     VK_ACCESS_2_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,   VK_IMAGE_LAYOUT_UNDEFINED },
        { ResourceState::CopyDst,           VK_PIPELINE_STAGE_2_TRANSFER_BIT,                   VK_ACCESS_2_TRANSFER_WRITE_BIT,                 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL },
        { ResourceState::CopySrc,           VK_PIPELINE_STAGE_2_TRANSFER_BIT,                   VK_ACCESS_2_TRANSFER_READ_BIT,                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL },
        { ResourceState::Present,           VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,               VK_ACCESS_2_MEMORY_READ_BIT,                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR },
        //{ ResourceState::AccelStructRead,   VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR | 
        //                                    VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,             VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR,VK_IMAGE_LAYOUT_UNDEFINED },
        //{ ResourceState::AccelStructWrite,  VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,VK_ACCESS_2_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,VK_IMAGE_LAYOUT_UNDEFINED },
        //{ ResourceState::AccelStructBuildInput,VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR,VK_IMAGE_LAYOUT_UNDEFINED },
        //{ ResourceState::AccelStructBuildBlas,VK_PIPELINE_STAGE_2_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,VK_ACCESS_2_ACCELERATION_STRUCTURE_READ_BIT_KHR,VK_IMAGE_LAYOUT_UNDEFINED },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // ImageDimensionMapping struct
    ////////////////////////////////////////////////////////////////////////////////////
    struct ImageDimensionMapping
    {
    public:
        ImageDimension Dimension;

        VkImageType VulkanImageType;
        VkImageViewType VulkanImageViewType;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ImageDimensionMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_ImageDimensionMappings = std::to_array<ImageDimensionMapping>({
        // Dimension                        ImageType           ImageViewType
        { ImageDimension::Unknown,          VK_IMAGE_TYPE_2D,   VK_IMAGE_VIEW_TYPE_2D }, // For proper indexing
        { ImageDimension::Image1D,          VK_IMAGE_TYPE_1D,   VK_IMAGE_VIEW_TYPE_1D },
        { ImageDimension::Image1DArray,     VK_IMAGE_TYPE_1D,   VK_IMAGE_VIEW_TYPE_1D_ARRAY },
        { ImageDimension::Image2D,          VK_IMAGE_TYPE_2D,   VK_IMAGE_VIEW_TYPE_2D },
        { ImageDimension::Image2DArray,     VK_IMAGE_TYPE_2D,   VK_IMAGE_VIEW_TYPE_2D_ARRAY },
        { ImageDimension::ImageCube,        VK_IMAGE_TYPE_2D,   VK_IMAGE_VIEW_TYPE_CUBE },
        { ImageDimension::ImageCubeArray,   VK_IMAGE_TYPE_2D,   VK_IMAGE_VIEW_TYPE_CUBE_ARRAY },
        { ImageDimension::Image2DMS,        VK_IMAGE_TYPE_2D,   VK_IMAGE_VIEW_TYPE_2D },
        { ImageDimension::Image2DMSArray,   VK_IMAGE_TYPE_2D,   VK_IMAGE_VIEW_TYPE_2D_ARRAY },
        { ImageDimension::Image3D,          VK_IMAGE_TYPE_3D,   VK_IMAGE_VIEW_TYPE_3D },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // FormatMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct FormatMapping
    {
    public:
        Format ImageFormat;

        VkFormat VulkanFormat;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // FormatMapping arrays
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_ImageFormatMapping = std::to_array<FormatMapping>({
        // ImageFormat              VulkanFormat
        { Format::Unknown,          VK_FORMAT_UNDEFINED                },
        { Format::R8UInt,           VK_FORMAT_R8_UINT                  },
        { Format::R8SInt,           VK_FORMAT_R8_SINT                  },
        { Format::R8Unorm,          VK_FORMAT_R8_UNORM                 },
        { Format::R8Snorm,          VK_FORMAT_R8_SNORM                 },
        { Format::RG8UInt,          VK_FORMAT_R8G8_UINT                },
        { Format::RG8SInt,          VK_FORMAT_R8G8_SINT                },
        { Format::RG8Unorm,         VK_FORMAT_R8G8_UNORM               },
        { Format::RG8Snorm,         VK_FORMAT_R8G8_SNORM               },
        { Format::R16UInt,          VK_FORMAT_R16_UINT                 },
        { Format::R16SInt,          VK_FORMAT_R16_SINT                 },
        { Format::R16Unorm,         VK_FORMAT_R16_UNORM                },
        { Format::R16Snorm,         VK_FORMAT_R16_SNORM                },
        { Format::R16Float,         VK_FORMAT_R16_SFLOAT               },
        { Format::BGRA4Unorm,       VK_FORMAT_B4G4R4A4_UNORM_PACK16   },
        { Format::B5G6R5Unorm,      VK_FORMAT_B5G6R5_UNORM_PACK16     },
        { Format::B5G5R5A1Unorm,    VK_FORMAT_B5G5R5A1_UNORM_PACK16   },
        { Format::RGBA8UInt,        VK_FORMAT_R8G8B8A8_UINT           },
        { Format::RGBA8SInt,        VK_FORMAT_R8G8B8A8_SINT           },
        { Format::RGBA8Unorm,       VK_FORMAT_R8G8B8A8_UNORM          },
        { Format::RGBA8Snorm,       VK_FORMAT_R8G8B8A8_SNORM          },
        { Format::BGRA8Unorm,       VK_FORMAT_B8G8R8A8_UNORM          },
        { Format::SRGBA8Unorm,      VK_FORMAT_R8G8B8A8_SRGB           },
        { Format::SBGRA8Unorm,      VK_FORMAT_B8G8R8A8_SRGB           },
        { Format::R10G10B10A2Unorm, VK_FORMAT_A2B10G10R10_UNORM_PACK32},
        { Format::R11G11B10Float,   VK_FORMAT_B10G11R11_UFLOAT_PACK32 },
        { Format::RG16UInt,         VK_FORMAT_R16G16_UINT             },
        { Format::RG16SInt,         VK_FORMAT_R16G16_SINT             },
        { Format::RG16Unorm,        VK_FORMAT_R16G16_UNORM            },
        { Format::RG16Snorm,        VK_FORMAT_R16G16_SNORM            },
        { Format::RG16Float,        VK_FORMAT_R16G16_SFLOAT           },
        { Format::R32UInt,          VK_FORMAT_R32_UINT                },
        { Format::R32SInt,          VK_FORMAT_R32_SINT                },
        { Format::R32Float,         VK_FORMAT_R32_SFLOAT              },
        { Format::RGBA16UInt,       VK_FORMAT_R16G16B16A16_UINT       },
        { Format::RGBA16SInt,       VK_FORMAT_R16G16B16A16_SINT       },
        { Format::RGBA16Float,      VK_FORMAT_R16G16B16A16_SFLOAT     },
        { Format::RGBA16Unorm,      VK_FORMAT_R16G16B16A16_UNORM      },
        { Format::RGBA16Snorm,      VK_FORMAT_R16G16B16A16_SNORM      },
        { Format::RG32UInt,         VK_FORMAT_R32G32_UINT             },
        { Format::RG32SInt,         VK_FORMAT_R32G32_SINT             },
        { Format::RG32Float,        VK_FORMAT_R32G32_SFLOAT           },
        { Format::RGB32UInt,        VK_FORMAT_R32G32B32_UINT          },
        { Format::RGB32SInt,        VK_FORMAT_R32G32B32_SINT          },
        { Format::RGB32Float,       VK_FORMAT_R32G32B32_SFLOAT        },
        { Format::RGBA32UInt,       VK_FORMAT_R32G32B32A32_UINT       },
        { Format::RGBA32SInt,       VK_FORMAT_R32G32B32A32_SINT       },
        { Format::RGBA32Float,      VK_FORMAT_R32G32B32A32_SFLOAT     },
        { Format::D16,              VK_FORMAT_D16_UNORM               },
        { Format::D24S8,            VK_FORMAT_D24_UNORM_S8_UINT       },
        { Format::D24S8,            VK_FORMAT_D24_UNORM_S8_UINT       },
        { Format::X24G8UInt,        VK_FORMAT_D24_UNORM_S8_UINT       }, 
        { Format::D32,              VK_FORMAT_D32_SFLOAT              },
        { Format::D32S8,            VK_FORMAT_D32_SFLOAT_S8_UINT      },
        { Format::D32S8,            VK_FORMAT_D32_SFLOAT_S8_UINT      }, 
        { Format::X32G8UInt,        VK_FORMAT_D32_SFLOAT_S8_UINT      }, 
        { Format::BC1Unorm,         VK_FORMAT_BC1_RGBA_UNORM_BLOCK    },
        { Format::BC1UnormSRGB,     VK_FORMAT_BC1_RGBA_SRGB_BLOCK     },
        { Format::BC2Unorm,         VK_FORMAT_BC2_UNORM_BLOCK         },
        { Format::BC2UnormSRGB,     VK_FORMAT_BC2_SRGB_BLOCK          },
        { Format::BC3Unorm,         VK_FORMAT_BC3_UNORM_BLOCK         },
        { Format::BC3UnormSRGB,     VK_FORMAT_BC3_SRGB_BLOCK          },
        { Format::BC4Unorm,         VK_FORMAT_BC4_UNORM_BLOCK         },
        { Format::BC4Snorm,         VK_FORMAT_BC4_SNORM_BLOCK         },
        { Format::BC5Unorm,         VK_FORMAT_BC5_UNORM_BLOCK         },
        { Format::BC5Snorm,         VK_FORMAT_BC5_SNORM_BLOCK         },
        { Format::BC6HUFloat,       VK_FORMAT_BC6H_UFLOAT_BLOCK       },
        { Format::BC6HSFloat,       VK_FORMAT_BC6H_SFLOAT_BLOCK       },
        { Format::BC7Unorm,         VK_FORMAT_BC7_UNORM_BLOCK         },
        { Format::BC7UnormSRGB,     VK_FORMAT_BC7_SRGB_BLOCK          },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // FilterMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct FilterMapping
    {
    public:
        FilterMode Filter;

        VkFilter VulkanFilter;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // FilterMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_FilterMapping = std::to_array<FilterMapping>({
        // FilterMode               VulkanFilter
        { FilterMode::None,         VK_FILTER_NEAREST },
        { FilterMode::Nearest,      VK_FILTER_NEAREST },
        { FilterMode::Linear,       VK_FILTER_LINEAR },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // SamplerAddressMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct SamplerAddressMapping
    {
    public:
        SamplerAddressMode AddressMode;

        VkSamplerAddressMode VulkanSamplerAddressMode;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // SamplerAddressMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_SamplerAddressMapping = std::to_array<SamplerAddressMapping>({
        // SamplerAddressMode                       VulkanSamplerAddressMode
        { SamplerAddressMode::ClampToEdge,          VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE },
        { SamplerAddressMode::Repeat,               VK_SAMPLER_ADDRESS_MODE_REPEAT },
        { SamplerAddressMode::ClampToBorder,        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER },
        { SamplerAddressMode::MirroredRepeat,       VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT },
        { SamplerAddressMode::MirrorClampToEdge,    VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // ColourSpaceMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct ColourSpaceMapping
    {
    public:
        ColourSpace Space;

        VkColorSpaceKHR VulkanColorSpace;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ColourSpaceMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_ColourSpaceMapping = std::to_array<ColourSpaceMapping>({
        // ColourSpace                  VulkanColorSpace
        { ColourSpace::SRGB,            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
        { ColourSpace::HDR,             VK_COLOR_SPACE_HDR10_ST2084_EXT },
        { ColourSpace::LinearSRGB,      VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT },
        { ColourSpace::DisplayNative,   VK_COLOR_SPACE_DISPLAY_NATIVE_AMD },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // LoadOperationMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct LoadOperationMapping
    {
    public:
        LoadOperation Operation;

        VkAttachmentLoadOp VulkanOperation;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // LoadOperationMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_LoadOperationMapping = std::to_array<LoadOperationMapping>({
        // LoadOperation            VulkanOperation
        { LoadOperation::None,      VK_ATTACHMENT_LOAD_OP_DONT_CARE },
        { LoadOperation::Clear,     VK_ATTACHMENT_LOAD_OP_CLEAR },
        { LoadOperation::Load,      VK_ATTACHMENT_LOAD_OP_LOAD },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // StoreOperationMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct StoreOperationMapping
    {
    public:
        StoreOperation Operation;

        VkAttachmentStoreOp VulkanOperation;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // LoadOperationMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_StoreOperationMapping = std::to_array<StoreOperationMapping>({
        // LoadOperation            VulkanOperation
        { StoreOperation::None,     VK_ATTACHMENT_STORE_OP_DONT_CARE },
        { StoreOperation::Store,    VK_ATTACHMENT_STORE_OP_STORE },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // ShaderStageMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct ShaderStageMapping
    {
    public:
        ShaderStage Stage;

        VkShaderStageFlags VulkanStage;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ShaderStageMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_ShaderStageMapping = std::to_array<ShaderStageMapping>({
        // Stage                                VulkanStage
        { ShaderStage::None,                    static_cast<VkShaderStageFlags>(0) },
        { ShaderStage::Vertex,                  VK_SHADER_STAGE_VERTEX_BIT },
        { ShaderStage::Fragment,                VK_SHADER_STAGE_FRAGMENT_BIT },
        { ShaderStage::Compute,                 VK_SHADER_STAGE_COMPUTE_BIT },
        { ShaderStage::Geometry,                VK_SHADER_STAGE_GEOMETRY_BIT },
        { ShaderStage::TesselationControl,      VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT },
        { ShaderStage::TesselationEvaluation,   VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT },
        { ShaderStage::Task,                    VK_SHADER_STAGE_TASK_BIT_EXT },
        { ShaderStage::Mesh,                    VK_SHADER_STAGE_MESH_BIT_EXT },
        { ShaderStage::AllGraphics,             VK_SHADER_STAGE_ALL_GRAPHICS },
        { ShaderStage::RayGeneration,           VK_SHADER_STAGE_RAYGEN_BIT_KHR },
        { ShaderStage::AnyHit,                  VK_SHADER_STAGE_ANY_HIT_BIT_KHR },
        { ShaderStage::ClosestHit,              VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR },
        { ShaderStage::Miss,                    VK_SHADER_STAGE_MISS_BIT_KHR },
        { ShaderStage::Intersection,            VK_SHADER_STAGE_INTERSECTION_BIT_KHR },
        { ShaderStage::Callable,                VK_SHADER_STAGE_CALLABLE_BIT_KHR },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // PipelineBindpointMapping 
    ////////////////////////////////////////////////////////////////////////////////////
    struct PipelineBindpointMapping
    {
    public:
        PipelineBindpoint Bindpoint;

        VkPipelineBindPoint VulkanBindpoint;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // PipelineBindPointMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_PipelineBindpointMapping = std::to_array<PipelineBindpointMapping>({
        // PipelineBindpoint                VulkanBindpoint
        { PipelineBindpoint::Graphics,      VK_PIPELINE_BIND_POINT_GRAPHICS },
        { PipelineBindpoint::Compute,       VK_PIPELINE_BIND_POINT_COMPUTE },
        { PipelineBindpoint::RayTracing,    VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // PrimitiveTypeMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct PrimitiveTypeMapping
    {
    public:
        PrimitiveType Type;

        VkPrimitiveTopology VulkanPrimitiveType;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // PrimitiveTypeMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_PrimitiveTypeMapping = std::to_array<PrimitiveTypeMapping>({
        // PrimitiveType                                VulkanPrimitiveType
        { PrimitiveType::PointList,                     VK_PRIMITIVE_TOPOLOGY_POINT_LIST },
        { PrimitiveType::LineList,                      VK_PRIMITIVE_TOPOLOGY_LINE_LIST },
        { PrimitiveType::LineStrip,                     VK_PRIMITIVE_TOPOLOGY_LINE_STRIP },
        { PrimitiveType::TriangleList,                  VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST },
        { PrimitiveType::TriangleStrip,                 VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP },
        { PrimitiveType::TriangleFan,                   VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN },
        { PrimitiveType::TriangleListWithAdjacency,     VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY },
        { PrimitiveType::TriangleStripWithAdjacency,    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY },
        { PrimitiveType::PatchList,                     VK_PRIMITIVE_TOPOLOGY_PATCH_LIST },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // BlendFactorMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct BlendFactorMapping
    {
    public:
        BlendFactor Factor;

        VkBlendFactor VulkanBlendFactor;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // BlendFactorMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_BlendFactorMapping = std::to_array<BlendFactorMapping>({
        // BlendFactor                          VulkanBlendFactor
        { BlendFactor::Zero,                    VK_BLEND_FACTOR_ZERO },
        { BlendFactor::One,                     VK_BLEND_FACTOR_ONE },
        { BlendFactor::SrcColour,               VK_BLEND_FACTOR_SRC_COLOR },
        { BlendFactor::OneMinusSrcColour,       VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR },
        { BlendFactor::OneMinusSrcColour,       VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR },
        { BlendFactor::SrcAlpha,                VK_BLEND_FACTOR_SRC_ALPHA },
        { BlendFactor::OneMinusSrcAlpha,        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA },
        { BlendFactor::DstAlpha,                VK_BLEND_FACTOR_DST_ALPHA },
        { BlendFactor::OneMinusDstAlpha,        VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA },
        { BlendFactor::DstColour,               VK_BLEND_FACTOR_DST_COLOR },
        { BlendFactor::OneMinusDstColour,       VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR },
        { BlendFactor::SrcAlphaSaturate,        VK_BLEND_FACTOR_SRC_ALPHA_SATURATE },
        { BlendFactor::ConstantColour,          VK_BLEND_FACTOR_CONSTANT_COLOR },
        { BlendFactor::OneMinusConstantColour,  VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR },
        { BlendFactor::Src1Colour,              VK_BLEND_FACTOR_SRC1_COLOR },
        { BlendFactor::OneMinusSrc1Colour,      VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR },
        { BlendFactor::Src1Alpha,               VK_BLEND_FACTOR_SRC1_ALPHA },
        { BlendFactor::OneMinusSrc1Alpha,       VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // BlendOperationMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct BlendOperationMapping
    {
    public:
        BlendOperation Operation;

        VkBlendOp VulkanBlendOperation;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // BlendOperationMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_BlendOperationMapping = std::to_array<BlendOperationMapping>({
        // BlendOperation                   VulkanBlendOperation
        { BlendOperation::Add,              VK_BLEND_OP_ADD },
        { BlendOperation::Subtract,         VK_BLEND_OP_SUBTRACT },
        { BlendOperation::ReverseSubtract,  VK_BLEND_OP_REVERSE_SUBTRACT },
        { BlendOperation::Min,              VK_BLEND_OP_MIN },
        { BlendOperation::Max,              VK_BLEND_OP_MAX },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // ColourMaskMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct ColourMaskMapping
    {
    public:
        ColourMask Mask;

        VkColorComponentFlags VulkanColourComponentFlags;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ColourMaskMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_ColourMaskMapping = std::to_array<ColourMaskMapping>({
        // ColourMask               VulkanColourComponentFlags
        { ColourMask::None,         static_cast<VkColorComponentFlags>(0) },
        { ColourMask::Red,          VK_COLOR_COMPONENT_R_BIT },
        { ColourMask::Green,        VK_COLOR_COMPONENT_G_BIT },
        { ColourMask::Blue,         VK_COLOR_COMPONENT_B_BIT },
        { ColourMask::Alpha,        VK_COLOR_COMPONENT_A_BIT },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // RasterFillModeMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct RasterFillModeMapping
    {
    public:
        RasterFillMode FillMode;

        VkPolygonMode VulkanPolygonMode;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // RasterFillModeMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_RasterFillModeMapping = std::to_array<RasterFillModeMapping>({
        // RasterFillMode               VulkanPolygonMode
        { RasterFillMode::Fill,         VK_POLYGON_MODE_FILL },
        { RasterFillMode::Line,         VK_POLYGON_MODE_LINE },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // RasterCullingModeMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct RasterCullingModeMapping
    {
    public:
        RasterCullingMode CullingMode;

        VkCullModeFlags VulkanCullModeFlags;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // RasterCullingModeMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_RasterCullingModeMapping = std::to_array<RasterCullingModeMapping>({
        // RasterCullingMode            VulkanCullModeFlags
        { RasterCullingMode::None,      VK_CULL_MODE_NONE },
        { RasterCullingMode::Back,      VK_CULL_MODE_BACK_BIT },
        { RasterCullingMode::Front,     VK_CULL_MODE_FRONT_BIT },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // StencilOperationMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct StencilOperationMapping
    {
    public:
        StencilOperation Operation;

        VkStencilOp VulkanStencilOperation;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // StencilOperationMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_StencilOperationMapping = std::to_array<StencilOperationMapping>({
        // StencilOperation                         VulkanStencilOperation
        { StencilOperation::Keep,                   VK_STENCIL_OP_KEEP },
        { StencilOperation::Zero,                   VK_STENCIL_OP_ZERO },
        { StencilOperation::Replace,                VK_STENCIL_OP_REPLACE },
        { StencilOperation::IncrementAndClamp,      VK_STENCIL_OP_INCREMENT_AND_CLAMP },
        { StencilOperation::DecrementAndClamp,      VK_STENCIL_OP_DECREMENT_AND_CLAMP },
        { StencilOperation::Invert,                 VK_STENCIL_OP_INVERT },
        { StencilOperation::IncrementAndWrap,       VK_STENCIL_OP_INCREMENT_AND_WRAP },
        { StencilOperation::DecrementAndWrap,       VK_STENCIL_OP_DECREMENT_AND_WRAP },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // ComparisonFuncMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct ComparisonFuncMapping
    {
    public:
        ComparisonFunc Comparison;

        VkCompareOp VulkanCompareOperation;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ComparisonFuncMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_ComparisonFuncMapping = std::to_array<ComparisonFuncMapping>({
        // ComparisonFunc                   VulkanCompareOperation
        { ComparisonFunc::Never,            VK_COMPARE_OP_NEVER },
        { ComparisonFunc::Less,             VK_COMPARE_OP_LESS },
        { ComparisonFunc::Equal,            VK_COMPARE_OP_EQUAL },
        { ComparisonFunc::LessOrEqual,      VK_COMPARE_OP_LESS_OR_EQUAL },
        { ComparisonFunc::Greater,          VK_COMPARE_OP_GREATER },
        { ComparisonFunc::NotEqual,         VK_COMPARE_OP_NOT_EQUAL },
        { ComparisonFunc::GreaterOrEqual,   VK_COMPARE_OP_GREATER_OR_EQUAL },
        { ComparisonFunc::Always,           VK_COMPARE_OP_ALWAYS },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // ResourceTypeMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct ResourceTypeMapping
    {
    public:
        ResourceType Type;

        VkDescriptorType VulkanDescriptorType;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ResourceTypeMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_ResourceTypeMapping = std::to_array<ResourceTypeMapping>({
        // ResourceType                         VulkanDescriptorType
        { ResourceType::None,                   static_cast<VkDescriptorType>(0) },
        { ResourceType::Image,                  VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE },
        { ResourceType::ImageUnordered,         VK_DESCRIPTOR_TYPE_STORAGE_IMAGE },
        { ResourceType::StorageBuffer,          VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
        { ResourceType::StorageBufferUnordered, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
        { ResourceType::DynamicStorageBuffer,   VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC },
        { ResourceType::UniformBuffer,          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
        { ResourceType::DynamicUniformBuffer,   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC },
        { ResourceType::Sampler,                VK_DESCRIPTOR_TYPE_SAMPLER },
        { ResourceType::PushConstants,          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }, // Note: Not really, but there is no PushConstants descriptor
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // Conversion helper methods
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr VkPipelineStageFlags2 ResourceStateToFirstVkPipelineStage(ResourceState state)
    {
        return g_ResourceStateMapping[static_cast<size_t>((std::to_underlying(state) ? (std::countr_zero(std::to_underlying(state)) + 1) : 0))].StageFlags;
    }

    inline constexpr VkPipelineStageFlags2 ResourceStateToVkPipelineStage(ResourceState state)
    {
        VkPipelineStageFlags2 result = 0;
        std::underlying_type_t<ResourceState> value = std::to_underlying(state);

        while (value) 
        {
            int index = std::countr_zero(value);
            value &= ~(1u << index); // clear bit

            result |= g_ResourceStateMapping[static_cast<size_t>(index) + 1].StageFlags;
        }

        return result;
    }

    inline constexpr VkAccessFlags2 ResourceStateToFirstVkAccessMask(ResourceState state)
    {
        return g_ResourceStateMapping[static_cast<size_t>((std::to_underlying(state) ? (std::countr_zero(std::to_underlying(state)) + 1) : 0))].AccessMask;
    }

    inline constexpr VkAccessFlags2 ResourceStateToVkAccessMask(ResourceState state)
    {
        VkAccessFlags2 result = 0;
        std::underlying_type_t<ResourceState> value = std::to_underlying(state);

        while (value)
        {
            int index = std::countr_zero(value);
            value &= ~(1u << index); // clear bit

            result |= g_ResourceStateMapping[static_cast<size_t>(index) + 1].AccessMask;
        }

        return result;
    }

    inline constexpr VkImageLayout ResourceStateToImageLayout(ResourceState state) // Note: We take the first image layout?
    {
        return g_ResourceStateMapping[static_cast<size_t>((std::to_underlying(state) ? (std::countr_zero(std::to_underlying(state)) + 1) : 0))].ImageLayout;
    }

    inline constexpr const ResourceStateMapping& ResourceStateToMapping(ResourceState state) { return g_ResourceStateMapping[static_cast<size_t>((std::to_underlying(state) ? (std::countr_zero(std::to_underlying(state)) + 1) : 0))]; }

    inline constexpr VkImageType ImageDimensionToVkImageType(ImageDimension dimension) { return g_ImageDimensionMappings[static_cast<size_t>(dimension)].VulkanImageType; }
    inline constexpr VkImageViewType ImageDimensionToVkImageViewType(ImageDimension dimension) { return g_ImageDimensionMappings[static_cast<size_t>(dimension)].VulkanImageViewType; }

    inline constexpr VkFormat FormatToVkFormat(Format format) { return g_ImageFormatMapping[static_cast<size_t>(format)].VulkanFormat; }
    inline constexpr Format VkFormatToFormat(VkFormat vkFormat)
    {
        switch (vkFormat)
        {
        case VK_FORMAT_UNDEFINED:                 return Format::Unknown;
        case VK_FORMAT_R8_UINT:                   return Format::R8UInt;
        case VK_FORMAT_R8_SINT:                   return Format::R8SInt;
        case VK_FORMAT_R8_UNORM:                  return Format::R8Unorm;
        case VK_FORMAT_R8_SNORM:                  return Format::R8Snorm;
        case VK_FORMAT_R8G8_UINT:                 return Format::RG8UInt;
        case VK_FORMAT_R8G8_SINT:                 return Format::RG8SInt;
        case VK_FORMAT_R8G8_UNORM:                return Format::RG8Unorm;
        case VK_FORMAT_R8G8_SNORM:                return Format::RG8Snorm;
        case VK_FORMAT_R16_UINT:                  return Format::R16UInt;
        case VK_FORMAT_R16_SINT:                  return Format::R16SInt;
        case VK_FORMAT_R16_UNORM:                 return Format::R16Unorm;
        case VK_FORMAT_R16_SNORM:                 return Format::R16Snorm;
        case VK_FORMAT_R16_SFLOAT:                return Format::R16Float;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:     return Format::BGRA4Unorm;
        case VK_FORMAT_B5G6R5_UNORM_PACK16:       return Format::B5G6R5Unorm;
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:     return Format::B5G5R5A1Unorm;
        case VK_FORMAT_R8G8B8A8_UINT:             return Format::RGBA8UInt;
        case VK_FORMAT_R8G8B8A8_SINT:             return Format::RGBA8SInt;
        case VK_FORMAT_R8G8B8A8_UNORM:            return Format::RGBA8Unorm;
        case VK_FORMAT_R8G8B8A8_SNORM:            return Format::RGBA8Snorm;
        case VK_FORMAT_B8G8R8A8_UNORM:            return Format::BGRA8Unorm;
        case VK_FORMAT_R8G8B8A8_SRGB:             return Format::SRGBA8Unorm;
        case VK_FORMAT_B8G8R8A8_SRGB:             return Format::SBGRA8Unorm;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:  return Format::R10G10B10A2Unorm;
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:   return Format::R11G11B10Float;
        case VK_FORMAT_R16G16_UINT:               return Format::RG16UInt;
        case VK_FORMAT_R16G16_SINT:               return Format::RG16SInt;
        case VK_FORMAT_R16G16_UNORM:              return Format::RG16Unorm;
        case VK_FORMAT_R16G16_SNORM:              return Format::RG16Snorm;
        case VK_FORMAT_R16G16_SFLOAT:             return Format::RG16Float;
        case VK_FORMAT_R32_UINT:                  return Format::R32UInt;
        case VK_FORMAT_R32_SINT:                  return Format::R32SInt;
        case VK_FORMAT_R32_SFLOAT:                return Format::R32Float;
        case VK_FORMAT_R16G16B16A16_UINT:         return Format::RGBA16UInt;
        case VK_FORMAT_R16G16B16A16_SINT:         return Format::RGBA16SInt;
        case VK_FORMAT_R16G16B16A16_SFLOAT:       return Format::RGBA16Float;
        case VK_FORMAT_R16G16B16A16_UNORM:        return Format::RGBA16Unorm;
        case VK_FORMAT_R16G16B16A16_SNORM:        return Format::RGBA16Snorm;
        case VK_FORMAT_R32G32_UINT:               return Format::RG32UInt;
        case VK_FORMAT_R32G32_SINT:               return Format::RG32SInt;
        case VK_FORMAT_R32G32_SFLOAT:             return Format::RG32Float;
        case VK_FORMAT_R32G32B32_UINT:            return Format::RGB32UInt;
        case VK_FORMAT_R32G32B32_SINT:            return Format::RGB32SInt;
        case VK_FORMAT_R32G32B32_SFLOAT:          return Format::RGB32Float;
        case VK_FORMAT_R32G32B32A32_UINT:         return Format::RGBA32UInt;
        case VK_FORMAT_R32G32B32A32_SINT:         return Format::RGBA32SInt;
        case VK_FORMAT_R32G32B32A32_SFLOAT:       return Format::RGBA32Float;

        case VK_FORMAT_D16_UNORM:                 return Format::D16;
        case VK_FORMAT_D24_UNORM_S8_UINT:         return Format::D24S8; // or Format::X24G8UInt;
        case VK_FORMAT_D32_SFLOAT:                return Format::D32;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:        return Format::D32S8; // or Format::X32G8UInt;

        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:      return Format::BC1Unorm;
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:       return Format::BC1UnormSRGB;
        case VK_FORMAT_BC2_UNORM_BLOCK:           return Format::BC2Unorm;
        case VK_FORMAT_BC2_SRGB_BLOCK:            return Format::BC2UnormSRGB;
        case VK_FORMAT_BC3_UNORM_BLOCK:           return Format::BC3Unorm;
        case VK_FORMAT_BC3_SRGB_BLOCK:            return Format::BC3UnormSRGB;
        case VK_FORMAT_BC4_UNORM_BLOCK:           return Format::BC4Unorm;
        case VK_FORMAT_BC4_SNORM_BLOCK:           return Format::BC4Snorm;
        case VK_FORMAT_BC5_UNORM_BLOCK:           return Format::BC5Unorm;
        case VK_FORMAT_BC5_SNORM_BLOCK:           return Format::BC5Snorm;
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:         return Format::BC6HUFloat;
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:         return Format::BC6HSFloat;
        case VK_FORMAT_BC7_UNORM_BLOCK:           return Format::BC7Unorm;
        case VK_FORMAT_BC7_SRGB_BLOCK:            return Format::BC7UnormSRGB;

        default:
            break;
        }

        NG_UNREACHABLE();
        return Format::Unknown;
    }

    inline constexpr VkFilter FilterModeToVkFilter(FilterMode filter) { return g_FilterMapping[static_cast<size_t>(filter)].VulkanFilter; }
    inline constexpr VkSamplerAddressMode SamplerAddressModeToVkSamplerAddressMode(SamplerAddressMode mode) { return g_SamplerAddressMapping[static_cast<size_t>(mode)].VulkanSamplerAddressMode; }

    inline constexpr VkColorSpaceKHR ColourSpaceToVkColorSpaceKHR(ColourSpace space) { return g_ColourSpaceMapping[static_cast<size_t>(space)].VulkanColorSpace; }
    inline constexpr ColourSpace VkColorSpaceKHRToColourSpace(VkColorSpaceKHR space)
    {
        switch (space)
        {
        case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:         return ColourSpace::SRGB;
        case VK_COLOR_SPACE_HDR10_ST2084_EXT:           return ColourSpace::HDR;
        case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:   return ColourSpace::LinearSRGB;
        case VK_COLOR_SPACE_DISPLAY_NATIVE_AMD:         return ColourSpace::DisplayNative;

        default:
            break;
        }

        NG_UNREACHABLE();
        return ColourSpace::SRGB;
    }

    inline constexpr VkAttachmentLoadOp LoadOperationToVkLoadOperation(LoadOperation operation) { return g_LoadOperationMapping[static_cast<size_t>(operation)].VulkanOperation; }
    inline constexpr VkAttachmentStoreOp StoreOperationToVkStoreOperation(StoreOperation operation) { return g_StoreOperationMapping[static_cast<size_t>(operation)].VulkanOperation; }

    inline constexpr VkShaderStageFlags ShaderStageToVkShaderStageFlags(ShaderStage stage) 
    { 
        VkShaderStageFlags result = 0;
        std::underlying_type_t<ShaderStage> value = std::to_underlying(stage);

        while (value)
        {
            int index = std::countr_zero(value);
            value &= ~(1u << index); // clear bit

            result |= g_ShaderStageMapping[static_cast<size_t>(index) + 1].VulkanStage;
        }

        return result;
    }

    inline constexpr VkPipelineBindPoint PipelineBindpointToVkBindpoint(PipelineBindpoint point) { return g_PipelineBindpointMapping[static_cast<size_t>(point)].VulkanBindpoint; }
    inline constexpr VkPrimitiveTopology PrimitiveTypeToVkPrimitiveTopology(PrimitiveType type) { return g_PrimitiveTypeMapping[static_cast<size_t>(type)].VulkanPrimitiveType; }
    inline constexpr VkBlendFactor BlendFactorToVkBlendFactor(BlendFactor factor) { return g_BlendFactorMapping[static_cast<size_t>(factor)].VulkanBlendFactor; }
    inline constexpr VkBlendOp BlendOperationToVkBlendOp(BlendOperation operation) { return g_BlendOperationMapping[static_cast<size_t>(operation)].VulkanBlendOperation; }
    
    inline constexpr VkColorComponentFlags ColourMaskToVkColorComponentFlags(ColourMask mask)
    {
        VkColorComponentFlags result = 0;
        std::underlying_type_t<ColourMask> value = std::to_underlying(mask);

        while (value)
        {
            int index = std::countr_zero(value);
            value &= ~(1u << index); // clear bit

            result |= g_ColourMaskMapping[static_cast<size_t>(index) + 1].VulkanColourComponentFlags;
        }

        return result;
    }

    inline constexpr VkPolygonMode RasterFillModeToVkPolygonMode(RasterFillMode mode) { return g_RasterFillModeMapping[static_cast<size_t>(mode)].VulkanPolygonMode; }
    inline constexpr VkCullModeFlags RasterCullingModeToVkCullModeFlags(RasterCullingMode mode) { return g_RasterCullingModeMapping[static_cast<size_t>(mode)].VulkanCullModeFlags; }

    inline constexpr VkStencilOp StencilOperationToVkStencilOp(StencilOperation operation) { return g_StencilOperationMapping[static_cast<size_t>(operation)].VulkanStencilOperation; }
    inline constexpr VkCompareOp ComparisonFuncToVkCompareOp(ComparisonFunc func) { return g_ComparisonFuncMapping[static_cast<size_t>(func)].VulkanCompareOperation; }

    inline constexpr VkDescriptorType ResourceTypeToVkDescriptorType(ResourceType type) { return g_ResourceTypeMapping[static_cast<size_t>(type)].VulkanDescriptorType; }
    
    inline constexpr VkImageUsageFlags ImageSpecificationToVkImageUsageFlags(const ImageSpecification& specs)
    {
        VkImageUsageFlags ret = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        if (specs.IsShaderResource)
            ret |= VK_IMAGE_USAGE_SAMPLED_BIT;

        if (specs.IsRenderTarget)
        {
            if (FormatHasDepth(specs.ImageFormat) || FormatHasStencil(specs.ImageFormat))
                ret |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            else
                ret |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }

        if (specs.IsUnorderedAccessed)
            ret |= VK_IMAGE_USAGE_STORAGE_BIT;

        return ret;
    }

    inline constexpr VkImageAspectFlags VkFormatToImageAspect(VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
            return VK_IMAGE_ASPECT_DEPTH_BIT;

        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_ASPECT_STENCIL_BIT;

        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

        default:
            break;
        }

        return VK_IMAGE_ASPECT_COLOR_BIT;
    }

    inline constexpr VkImageAspectFlags GuessSubresourceImageAspectFlags(VkFormat format, ImageSubresourceViewType viewType)
    {
        VkImageAspectFlags flags = VkFormatToImageAspect(format);

        // If it has both depth and stencil check if that's what the viewType requests
        if ((flags & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
        {
            if (viewType == ImageSubresourceViewType::DepthOnly)
                flags = flags & (~VK_IMAGE_ASPECT_STENCIL_BIT);
            else if (viewType == ImageSubresourceViewType::StencilOnly)
                flags = flags & (~VK_IMAGE_ASPECT_DEPTH_BIT);
        }

        return flags;
    }

    inline constexpr VkSampleCountFlags SampleCountToVkSampleCountFlags(uint32_t sampleCount)
    {
        switch (sampleCount)
        {
        case 1:				return VK_SAMPLE_COUNT_1_BIT;
        case 2:				return VK_SAMPLE_COUNT_2_BIT;
        case 4:				return VK_SAMPLE_COUNT_4_BIT;
        case 8:				return VK_SAMPLE_COUNT_8_BIT;
        case 16:			return VK_SAMPLE_COUNT_16_BIT;
        case 32:			return VK_SAMPLE_COUNT_32_BIT;
        case 64:			return VK_SAMPLE_COUNT_64_BIT;

        default:
            break;
        }

        NG_UNREACHABLE();
        return VK_SAMPLE_COUNT_1_BIT;
    }

    inline constexpr VkBorderColor Vec4ToBorderColor(const Nano::Graphics::Maths::Vec4<float>& col)
    {
        if (col.r == 0.f && col.g == 0.f && col.b == 0.f)
        {
            if (col.a == 0.f)
                return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            else if (col.a == 1.f)
                return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        }
        else if (col.r == 1.f && col.g == 1.f && col.b == 1.f)
        {
            if (col.a == 1.f)
                return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        }

        NG_UNREACHABLE();
        return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    }

    inline constexpr VkPipelineStageFlags2 GetFirstPipelineStage(VkPipelineStageFlags2 stage)
    {
        return stage & (~stage + 1);
    }

    constexpr uint8_t GetVkPipelineStageOrder(VkPipelineStageFlags2 stage) 
    {
        switch (stage) 
        {
        case VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT:                       return 0;
        case VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT:                     return 1;
        case VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT:                      return 2;
        case VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT:                     return 3;
        case VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT:       return 4;
        case VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT:    return 5;
        case VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT:                   return 6;
        case VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT:                   return 7;
        case VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT:              return 8;
        case VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT:               return 9;
        case VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT:           return 10;
        case VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT:                    return 11;
        case VK_PIPELINE_STAGE_2_TRANSFER_BIT:                          return 12;
        case VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT:                    return 13;
        case VK_PIPELINE_STAGE_2_HOST_BIT:                              return 14;
        case VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT:                      return 15;
        case VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT:                      return 16;
        
        default: 
            break;
        }

        return std::numeric_limits<uint8_t>::max();
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Resolving methods
    ////////////////////////////////////////////////////////////////////////////////////
    ImageSliceSpecification ResolveImageSlice(const ImageSliceSpecification& sliceSpec, const ImageSpecification& imageSpec);
    ImageSubresourceSpecification ResolveImageSubresouce(const ImageSubresourceSpecification& subresourceSpec, const ImageSpecification& imageSpec, bool singleMip);

    BufferRange ResolveBufferRange(const BufferRange& range, const BufferSpecification& specs);
#endif

}