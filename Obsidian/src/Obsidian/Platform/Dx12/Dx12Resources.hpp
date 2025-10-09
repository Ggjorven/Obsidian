#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Maths/Structs.hpp"

#include "Obsidian/Renderer/ResourceSpec.hpp"
#include "Obsidian/Renderer/ImageSpec.hpp"
#include "Obsidian/Renderer/BindingsSpec.hpp"
#include "Obsidian/Renderer/PipelineSpec.hpp"
#include "Obsidian/Renderer/SwapchainSpec.hpp"

#include "Obsidian/Platform/Dx12/Dx12.hpp"
#include "Obsidian/Platform/Dx12/Dx12Descriptors.hpp"

#include <vector>

namespace Obsidian
{
    class Device;
}

namespace Obsidian::Internal
{

    class Dx12Device;

#if defined(OB_API_DX12)
    ////////////////////////////////////////////////////////////////////////////////////
    // ResourceStateMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct ResourceStateMapping
    {
    public:
        ResourceState State;

        D3D12_RESOURCE_STATES D3D12States;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ResourceStateMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_ResourceStateMapping = std::to_array<ResourceStateMapping>({
        // State                            D3D12States
        { ResourceState::Common,            D3D12_RESOURCE_STATE_COMMON },
        { ResourceState::StorageBuffer,     D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER },
        { ResourceState::VertexBuffer,      D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER },
        { ResourceState::IndexBuffer,       D3D12_RESOURCE_STATE_INDEX_BUFFER },
        { ResourceState::IndirectArgument,  D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT },
        { ResourceState::ShaderResource,    D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE },
        { ResourceState::UnorderedAccess,   D3D12_RESOURCE_STATE_UNORDERED_ACCESS },
        { ResourceState::RenderTarget,      D3D12_RESOURCE_STATE_RENDER_TARGET },
        { ResourceState::DepthWrite,        D3D12_RESOURCE_STATE_DEPTH_WRITE },
        { ResourceState::DepthRead,         D3D12_RESOURCE_STATE_DEPTH_READ },
        { ResourceState::CopyDst,           D3D12_RESOURCE_STATE_COPY_DEST },
        { ResourceState::CopySrc,           D3D12_RESOURCE_STATE_COPY_SOURCE },
        { ResourceState::Present,           D3D12_RESOURCE_STATE_PRESENT }
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // ImageDimensionMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct ImageDimensionMapping
    {
    public:
        ImageDimension Dimension;

        D3D12_RESOURCE_DIMENSION D3D12Dimension;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ImageDimensionMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_ImageDimensionMapping = std::to_array<ImageDimensionMapping>({
        // Dimension                        D3D12Dimension
        { ImageDimension::Unknown,          D3D12_RESOURCE_DIMENSION_TEXTURE1D },
        { ImageDimension::Image1D,          D3D12_RESOURCE_DIMENSION_TEXTURE1D },
        { ImageDimension::Image1DArray,     D3D12_RESOURCE_DIMENSION_TEXTURE1D },
        { ImageDimension::Image2D,          D3D12_RESOURCE_DIMENSION_TEXTURE2D },
        { ImageDimension::Image2DArray,     D3D12_RESOURCE_DIMENSION_TEXTURE2D },
        { ImageDimension::ImageCube,        D3D12_RESOURCE_DIMENSION_TEXTURE2D },
        { ImageDimension::ImageCubeArray,   D3D12_RESOURCE_DIMENSION_TEXTURE2D },
        { ImageDimension::Image2DMS,        D3D12_RESOURCE_DIMENSION_TEXTURE2D },
        { ImageDimension::Image2DMSArray,   D3D12_RESOURCE_DIMENSION_TEXTURE2D },
        { ImageDimension::Image3D,          D3D12_RESOURCE_DIMENSION_TEXTURE3D }
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // FilterModeMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct FilterModeMapping
    {
    public:
        FilterMode Filter;

        D3D12_FILTER_TYPE D3D12FilterType;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // FilterModeMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_FilterModeMapping = std::to_array<FilterModeMapping>({
        // Filter                       D3D12FilterType
        { FilterMode::None,             D3D12_FILTER_TYPE_POINT },
        { FilterMode::Nearest,          D3D12_FILTER_TYPE_POINT },
        { FilterMode::Linear,           D3D12_FILTER_TYPE_LINEAR },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // SamplerAddressModeMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct SamplerAddressModeMapping
    {
    public:
        SamplerAddressMode AddressMode;

        D3D12_TEXTURE_ADDRESS_MODE D3D12TextureAddressMode;
    };
    
    ////////////////////////////////////////////////////////////////////////////////////
    // SamplerAddressModeMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_SamplerAddressModeMapping = std::to_array<SamplerAddressModeMapping>({
        // AddressMode                      D3D12TextureAddressMode
        { SamplerAddressMode::Clamp,        D3D12_TEXTURE_ADDRESS_MODE_CLAMP },
        { SamplerAddressMode::Wrap,         D3D12_TEXTURE_ADDRESS_MODE_WRAP },
        { SamplerAddressMode::Border,       D3D12_TEXTURE_ADDRESS_MODE_BORDER },
        { SamplerAddressMode::Mirror,       D3D12_TEXTURE_ADDRESS_MODE_MIRROR },
        { SamplerAddressMode::MirrorOnce,   D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE }
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // SamplerReductionTypeMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct SamplerReductionTypeMapping
    {
    public:
        SamplerReductionType ReductionType;

        D3D12_FILTER_REDUCTION_TYPE D3D12ReductionType;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // SamplerReductionTypeMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_SamplerReductionTypeMapping = std::to_array<SamplerReductionTypeMapping>({
        // ReductionType                    D3D12ReductionType
        { SamplerReductionType::Standard,   D3D12_FILTER_REDUCTION_TYPE_STANDARD },
        { SamplerReductionType::Comparison, D3D12_FILTER_REDUCTION_TYPE_COMPARISON },
        { SamplerReductionType::Minimum,    D3D12_FILTER_REDUCTION_TYPE_MINIMUM },
        { SamplerReductionType::Maximum,    D3D12_FILTER_REDUCTION_TYPE_MAXIMUM }
    });

	////////////////////////////////////////////////////////////////////////////////////
	// FormatMapping
	////////////////////////////////////////////////////////////////////////////////////
    struct FormatMapping
    {
    public:
        Format AbstractFormat;
        
        DXGI_FORMAT ResourceFormat;
        DXGI_FORMAT SRVFormat;
        DXGI_FORMAT RTVFormat;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // FormatMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_FormatMapping = std::to_array<FormatMapping>({
        // AbstractFormat               ResourceFormat                      SRVFormat                               RTVFormat
        { Format::Unknown,              DXGI_FORMAT_UNKNOWN,                DXGI_FORMAT_UNKNOWN,                    DXGI_FORMAT_UNKNOWN                },

        { Format::R8UInt,               DXGI_FORMAT_R8_TYPELESS,            DXGI_FORMAT_R8_UINT,                    DXGI_FORMAT_R8_UINT                },
        { Format::R8SInt,               DXGI_FORMAT_R8_TYPELESS,            DXGI_FORMAT_R8_SINT,                    DXGI_FORMAT_R8_SINT                },
        { Format::R8Unorm,              DXGI_FORMAT_R8_TYPELESS,            DXGI_FORMAT_R8_UNORM,                   DXGI_FORMAT_R8_UNORM               },
        { Format::R8Snorm,              DXGI_FORMAT_R8_TYPELESS,            DXGI_FORMAT_R8_SNORM,                   DXGI_FORMAT_R8_SNORM               },
        { Format::RG8UInt,              DXGI_FORMAT_R8G8_TYPELESS,          DXGI_FORMAT_R8G8_UINT,                  DXGI_FORMAT_R8G8_UINT              },
        { Format::RG8SInt,              DXGI_FORMAT_R8G8_TYPELESS,          DXGI_FORMAT_R8G8_SINT,                  DXGI_FORMAT_R8G8_SINT              },
        { Format::RG8Unorm,             DXGI_FORMAT_R8G8_TYPELESS,          DXGI_FORMAT_R8G8_UNORM,                 DXGI_FORMAT_R8G8_UNORM             },
        { Format::RG8Snorm,             DXGI_FORMAT_R8G8_TYPELESS,          DXGI_FORMAT_R8G8_SNORM,                 DXGI_FORMAT_R8G8_SNORM             },
        { Format::R16UInt,              DXGI_FORMAT_R16_TYPELESS,           DXGI_FORMAT_R16_UINT,                   DXGI_FORMAT_R16_UINT               },
        { Format::R16SInt,              DXGI_FORMAT_R16_TYPELESS,           DXGI_FORMAT_R16_SINT,                   DXGI_FORMAT_R16_SINT               },
        { Format::R16Unorm,             DXGI_FORMAT_R16_TYPELESS,           DXGI_FORMAT_R16_UNORM,                  DXGI_FORMAT_R16_UNORM              },
        { Format::R16Snorm,             DXGI_FORMAT_R16_TYPELESS,           DXGI_FORMAT_R16_SNORM,                  DXGI_FORMAT_R16_SNORM              },
        { Format::R16Float,             DXGI_FORMAT_R16_TYPELESS,           DXGI_FORMAT_R16_FLOAT,                  DXGI_FORMAT_R16_FLOAT              },
        { Format::BGRA4Unorm,           DXGI_FORMAT_B4G4R4A4_UNORM,         DXGI_FORMAT_B4G4R4A4_UNORM,             DXGI_FORMAT_B4G4R4A4_UNORM         },
        { Format::B5G6R5Unorm,          DXGI_FORMAT_B5G6R5_UNORM,           DXGI_FORMAT_B5G6R5_UNORM,               DXGI_FORMAT_B5G6R5_UNORM           },
        { Format::B5G5R5A1Unorm,        DXGI_FORMAT_B5G5R5A1_UNORM,         DXGI_FORMAT_B5G5R5A1_UNORM,             DXGI_FORMAT_B5G5R5A1_UNORM         },
        { Format::RGBA8UInt,            DXGI_FORMAT_R8G8B8A8_TYPELESS,      DXGI_FORMAT_R8G8B8A8_UINT,              DXGI_FORMAT_R8G8B8A8_UINT          },
        { Format::RGBA8SInt,            DXGI_FORMAT_R8G8B8A8_TYPELESS,      DXGI_FORMAT_R8G8B8A8_SINT,              DXGI_FORMAT_R8G8B8A8_SINT          },
        { Format::RGBA8Unorm,           DXGI_FORMAT_R8G8B8A8_TYPELESS,      DXGI_FORMAT_R8G8B8A8_UNORM,             DXGI_FORMAT_R8G8B8A8_UNORM         },
        { Format::RGBA8Snorm,           DXGI_FORMAT_R8G8B8A8_TYPELESS,      DXGI_FORMAT_R8G8B8A8_SNORM,             DXGI_FORMAT_R8G8B8A8_SNORM         },
        { Format::BGRA8Unorm,           DXGI_FORMAT_B8G8R8A8_TYPELESS,      DXGI_FORMAT_B8G8R8A8_UNORM,             DXGI_FORMAT_B8G8R8A8_UNORM         },
        { Format::SRGBA8Unorm,          DXGI_FORMAT_R8G8B8A8_TYPELESS,      DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB    },
        { Format::SBGRA8Unorm,          DXGI_FORMAT_B8G8R8A8_TYPELESS,      DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,        DXGI_FORMAT_B8G8R8A8_UNORM_SRGB    },
        { Format::R10G10B10A2Unorm,     DXGI_FORMAT_R10G10B10A2_TYPELESS,   DXGI_FORMAT_R10G10B10A2_UNORM,          DXGI_FORMAT_R10G10B10A2_UNORM      },
        { Format::R11G11B10Float,       DXGI_FORMAT_R11G11B10_FLOAT,        DXGI_FORMAT_R11G11B10_FLOAT,            DXGI_FORMAT_R11G11B10_FLOAT        },
        { Format::RG16UInt,             DXGI_FORMAT_R16G16_TYPELESS,        DXGI_FORMAT_R16G16_UINT,                DXGI_FORMAT_R16G16_UINT            },
        { Format::RG16SInt,             DXGI_FORMAT_R16G16_TYPELESS,        DXGI_FORMAT_R16G16_SINT,                DXGI_FORMAT_R16G16_SINT            },
        { Format::RG16Unorm,            DXGI_FORMAT_R16G16_TYPELESS,        DXGI_FORMAT_R16G16_UNORM,               DXGI_FORMAT_R16G16_UNORM           },
        { Format::RG16Snorm,            DXGI_FORMAT_R16G16_TYPELESS,        DXGI_FORMAT_R16G16_SNORM,               DXGI_FORMAT_R16G16_SNORM           },
        { Format::RG16Float,            DXGI_FORMAT_R16G16_TYPELESS,        DXGI_FORMAT_R16G16_FLOAT,               DXGI_FORMAT_R16G16_FLOAT           },
        { Format::R32UInt,              DXGI_FORMAT_R32_TYPELESS,           DXGI_FORMAT_R32_UINT,                   DXGI_FORMAT_R32_UINT               },
        { Format::R32SInt,              DXGI_FORMAT_R32_TYPELESS,           DXGI_FORMAT_R32_SINT,                   DXGI_FORMAT_R32_SINT               },
        { Format::R32Float,             DXGI_FORMAT_R32_TYPELESS,           DXGI_FORMAT_R32_FLOAT,                  DXGI_FORMAT_R32_FLOAT              },
        { Format::RGBA16UInt,           DXGI_FORMAT_R16G16B16A16_TYPELESS,  DXGI_FORMAT_R16G16B16A16_UINT,          DXGI_FORMAT_R16G16B16A16_UINT      },
        { Format::RGBA16SInt,           DXGI_FORMAT_R16G16B16A16_TYPELESS,  DXGI_FORMAT_R16G16B16A16_SINT,          DXGI_FORMAT_R16G16B16A16_SINT      },
        { Format::RGBA16Float,          DXGI_FORMAT_R16G16B16A16_TYPELESS,  DXGI_FORMAT_R16G16B16A16_FLOAT,         DXGI_FORMAT_R16G16B16A16_FLOAT     },
        { Format::RGBA16Unorm,          DXGI_FORMAT_R16G16B16A16_TYPELESS,  DXGI_FORMAT_R16G16B16A16_UNORM,         DXGI_FORMAT_R16G16B16A16_UNORM     },
        { Format::RGBA16Snorm,          DXGI_FORMAT_R16G16B16A16_TYPELESS,  DXGI_FORMAT_R16G16B16A16_SNORM,         DXGI_FORMAT_R16G16B16A16_SNORM     },
        { Format::RG32UInt,             DXGI_FORMAT_R32G32_TYPELESS,        DXGI_FORMAT_R32G32_UINT,                DXGI_FORMAT_R32G32_UINT            },
        { Format::RG32SInt,             DXGI_FORMAT_R32G32_TYPELESS,        DXGI_FORMAT_R32G32_SINT,                DXGI_FORMAT_R32G32_SINT            },
        { Format::RG32Float,            DXGI_FORMAT_R32G32_TYPELESS,        DXGI_FORMAT_R32G32_FLOAT,               DXGI_FORMAT_R32G32_FLOAT           },
        { Format::RGB32UInt,            DXGI_FORMAT_R32G32B32_TYPELESS,     DXGI_FORMAT_R32G32B32_UINT,             DXGI_FORMAT_R32G32B32_UINT         },
        { Format::RGB32SInt,            DXGI_FORMAT_R32G32B32_TYPELESS,     DXGI_FORMAT_R32G32B32_SINT,             DXGI_FORMAT_R32G32B32_SINT         },
        { Format::RGB32Float,           DXGI_FORMAT_R32G32B32_TYPELESS,     DXGI_FORMAT_R32G32B32_FLOAT,            DXGI_FORMAT_R32G32B32_FLOAT        },
        { Format::RGBA32UInt,           DXGI_FORMAT_R32G32B32A32_TYPELESS,  DXGI_FORMAT_R32G32B32A32_UINT,          DXGI_FORMAT_R32G32B32A32_UINT      },
        { Format::RGBA32SInt,           DXGI_FORMAT_R32G32B32A32_TYPELESS,  DXGI_FORMAT_R32G32B32A32_SINT,          DXGI_FORMAT_R32G32B32A32_SINT      },
        { Format::RGBA32Float,          DXGI_FORMAT_R32G32B32A32_TYPELESS,  DXGI_FORMAT_R32G32B32A32_FLOAT,         DXGI_FORMAT_R32G32B32A32_FLOAT     },

        { Format::D16,                  DXGI_FORMAT_R16_TYPELESS,           DXGI_FORMAT_R16_UNORM,                  DXGI_FORMAT_D16_UNORM              },
        { Format::D24S8,                DXGI_FORMAT_R24G8_TYPELESS,         DXGI_FORMAT_R24_UNORM_X8_TYPELESS,      DXGI_FORMAT_D24_UNORM_S8_UINT      },
        { Format::X24G8UInt,            DXGI_FORMAT_R24G8_TYPELESS,         DXGI_FORMAT_X24_TYPELESS_G8_UINT,       DXGI_FORMAT_D24_UNORM_S8_UINT      },
        { Format::D32,                  DXGI_FORMAT_R32_TYPELESS,           DXGI_FORMAT_R32_FLOAT,                  DXGI_FORMAT_D32_FLOAT              },
        { Format::D32S8,                DXGI_FORMAT_R32G8X24_TYPELESS,      DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,   DXGI_FORMAT_D32_FLOAT_S8X24_UINT   },
        { Format::X32G8UInt,            DXGI_FORMAT_R32G8X24_TYPELESS,      DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,    DXGI_FORMAT_D32_FLOAT_S8X24_UINT   },

        { Format::BC1Unorm,             DXGI_FORMAT_BC1_TYPELESS,           DXGI_FORMAT_BC1_UNORM,                  DXGI_FORMAT_BC1_UNORM              },
        { Format::BC1UnormSRGB,         DXGI_FORMAT_BC1_TYPELESS,           DXGI_FORMAT_BC1_UNORM_SRGB,             DXGI_FORMAT_BC1_UNORM_SRGB         },
        { Format::BC2Unorm,             DXGI_FORMAT_BC2_TYPELESS,           DXGI_FORMAT_BC2_UNORM,                  DXGI_FORMAT_BC2_UNORM              },
        { Format::BC2UnormSRGB,         DXGI_FORMAT_BC2_TYPELESS,           DXGI_FORMAT_BC2_UNORM_SRGB,             DXGI_FORMAT_BC2_UNORM_SRGB         },
        { Format::BC3Unorm,             DXGI_FORMAT_BC3_TYPELESS,           DXGI_FORMAT_BC3_UNORM,                  DXGI_FORMAT_BC3_UNORM              },
        { Format::BC3UnormSRGB,         DXGI_FORMAT_BC3_TYPELESS,           DXGI_FORMAT_BC3_UNORM_SRGB,             DXGI_FORMAT_BC3_UNORM_SRGB         },
        { Format::BC4Unorm,             DXGI_FORMAT_BC4_TYPELESS,           DXGI_FORMAT_BC4_UNORM,                  DXGI_FORMAT_BC4_UNORM              },
        { Format::BC4Snorm,             DXGI_FORMAT_BC4_TYPELESS,           DXGI_FORMAT_BC4_SNORM,                  DXGI_FORMAT_BC4_SNORM              },
        { Format::BC5Unorm,             DXGI_FORMAT_BC5_TYPELESS,           DXGI_FORMAT_BC5_UNORM,                  DXGI_FORMAT_BC5_UNORM              },
        { Format::BC5Snorm,             DXGI_FORMAT_BC5_TYPELESS,           DXGI_FORMAT_BC5_SNORM,                  DXGI_FORMAT_BC5_SNORM              },
        { Format::BC6HUFloat,           DXGI_FORMAT_BC6H_TYPELESS,          DXGI_FORMAT_BC6H_UF16,                  DXGI_FORMAT_BC6H_UF16              },
        { Format::BC6HSFloat,           DXGI_FORMAT_BC6H_TYPELESS,          DXGI_FORMAT_BC6H_SF16,                  DXGI_FORMAT_BC6H_SF16              },
        { Format::BC7Unorm,             DXGI_FORMAT_BC7_TYPELESS,           DXGI_FORMAT_BC7_UNORM,                  DXGI_FORMAT_BC7_UNORM              },
        { Format::BC7UnormSRGB,         DXGI_FORMAT_BC7_TYPELESS,           DXGI_FORMAT_BC7_UNORM_SRGB,             DXGI_FORMAT_BC7_UNORM_SRGB         },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // ColourSpaceMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct ColourSpaceMapping
    {
    public:
        ColourSpace Space;

        DXGI_COLOR_SPACE_TYPE SpaceType;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ColourSpaceMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_ColourSpaceMapping = std::to_array<ColourSpaceMapping>({
        // Space                        SpaceType
        { ColourSpace::SRGB,            DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709 },
        { ColourSpace::HDR,             DXGI_COLOR_SPACE_RGB_STUDIO_G2084_NONE_P2020 },
        { ColourSpace::LinearSRGB,      DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709 },
        { ColourSpace::DisplayNative,   DXGI_COLOR_SPACE_CUSTOM }
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // ShaderStageMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct ShaderStageMapping
    {
    public:
        ShaderStage Stage;

        D3D12_SHADER_VISIBILITY Visibility;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ShaderStageMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_ShaderStageMapping = std::to_array<ShaderStageMapping>({
        // Stage                        Visibility
        { ShaderStage::None,            D3D12_SHADER_VISIBILITY_ALL }, // Default
        { ShaderStage::Vertex,          D3D12_SHADER_VISIBILITY_VERTEX },
        { ShaderStage::Pixel,           D3D12_SHADER_VISIBILITY_PIXEL },
        { ShaderStage::Compute,         D3D12_SHADER_VISIBILITY_ALL }, // Placeholder
        { ShaderStage::Geometry,        D3D12_SHADER_VISIBILITY_PIXEL },
        { ShaderStage::Hull,            D3D12_SHADER_VISIBILITY_HULL },
        { ShaderStage::Domain,          D3D12_SHADER_VISIBILITY_DOMAIN },
        { ShaderStage::Amplification,   D3D12_SHADER_VISIBILITY_AMPLIFICATION },
        { ShaderStage::Mesh,            D3D12_SHADER_VISIBILITY_MESH },
        { ShaderStage::AllGraphics,     D3D12_SHADER_VISIBILITY_ALL } // Placeholder

        // Note: Other shaderstages are not supported
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // PrimitiveTypeToD3D12TypeMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct PrimitiveTypeToD3D12TypeMapping
    {
    public:
        PrimitiveType PrimType;

        D3D12_PRIMITIVE_TOPOLOGY_TYPE D3D12Type;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // PrimiteTypeToD3D12TypeMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_PrimitiveTypeToD3D12TypeMapping = std::to_array<PrimitiveTypeToD3D12TypeMapping>({
        // PrimType                                     D3D12Type
        { PrimitiveType::PointList,                     D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT },
        { PrimitiveType::LineList,                      D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE },
        { PrimitiveType::LineStrip,                     D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE },
        { PrimitiveType::TriangleList,                  D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE },
        { PrimitiveType::TriangleStrip,                 D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE },
        { PrimitiveType::TriangleFan,                   D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE },
        { PrimitiveType::TriangleListWithAdjacency,     D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE },
        { PrimitiveType::TriangleStripWithAdjacency,    D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE },
        { PrimitiveType::PatchList,                     D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH }
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // PrimitiveTypeToD3DTypeMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct PrimitiveTypeToD3DTypeMapping
    {
    public:
        PrimitiveType PrimType;

        D3D_PRIMITIVE_TOPOLOGY D3DType;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // PrimitiveTypeToD3DTypeMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_PrimitiveTypeToD3DTypeMapping = std::to_array<PrimitiveTypeToD3DTypeMapping>({
        // PrimType                                     D3DType
        { PrimitiveType::PointList,                     D3D_PRIMITIVE_TOPOLOGY_POINTLIST },
        { PrimitiveType::LineList,                      D3D_PRIMITIVE_TOPOLOGY_LINELIST },
        { PrimitiveType::LineStrip,                     D3D_PRIMITIVE_TOPOLOGY_LINESTRIP },
        { PrimitiveType::TriangleList,                  D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST },
        { PrimitiveType::TriangleStrip,                 D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP },
        { PrimitiveType::TriangleFan,                   D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN },
        { PrimitiveType::TriangleListWithAdjacency,     D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ },
        { PrimitiveType::TriangleStripWithAdjacency,    D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST },
        { PrimitiveType::PatchList,                     D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST }
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // BlendFactorMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct BlendFactorMapping
    {
    public:
        BlendFactor Factor;

        D3D12_BLEND Blend;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // BlendFactorMapping
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_BlendFactorMapping = std::to_array<BlendFactorMapping>({
        // Factor                           Blend
        { BlendFactor::Zero,                D3D12_BLEND_ZERO },
        { BlendFactor::One,                 D3D12_BLEND_ONE },
        { BlendFactor::SrcColour,           D3D12_BLEND_SRC_COLOR },
        { BlendFactor::InvSrcColour,        D3D12_BLEND_INV_SRC_COLOR },
        { BlendFactor::SrcAlpha,            D3D12_BLEND_SRC_ALPHA },
        { BlendFactor::InvSrcAlpha,         D3D12_BLEND_INV_SRC_ALPHA },
        { BlendFactor::DstAlpha,            D3D12_BLEND_DEST_ALPHA },
        { BlendFactor::InvDstAlpha,         D3D12_BLEND_INV_DEST_ALPHA },
        { BlendFactor::DstColour,           D3D12_BLEND_DEST_COLOR },
        { BlendFactor::InvDstColour,        D3D12_BLEND_INV_DEST_COLOR },
        { BlendFactor::SrcAlphaSaturate,    D3D12_BLEND_SRC_ALPHA_SAT },
        { BlendFactor::ConstantColour,      D3D12_BLEND_BLEND_FACTOR },
        { BlendFactor::ConstantColour,      D3D12_BLEND_BLEND_FACTOR },
        { BlendFactor::InvConstantColour,   D3D12_BLEND_INV_BLEND_FACTOR },
        { BlendFactor::Src1Colour,          D3D12_BLEND_SRC1_COLOR },
        { BlendFactor::InvSrc1Colour,       D3D12_BLEND_INV_SRC1_COLOR },
        { BlendFactor::Src1Alpha,           D3D12_BLEND_SRC1_ALPHA },
        { BlendFactor::InvSrc1Alpha,        D3D12_BLEND_INV_SRC1_ALPHA }
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // BlendOperationMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct BlendOperationMapping
    {
    public:
        BlendOperation Operation;

        D3D12_BLEND_OP BlendOp;
    };
    
    ////////////////////////////////////////////////////////////////////////////////////
    // BlendOperationMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_BlendOperationMapping = std::to_array<BlendOperationMapping>({
        // Operation                        BlendOp
        { BlendOperation::Add,              D3D12_BLEND_OP_ADD },
        { BlendOperation::Subtract,         D3D12_BLEND_OP_SUBTRACT },
        { BlendOperation::ReverseSubtract,  D3D12_BLEND_OP_REV_SUBTRACT },
        { BlendOperation::Min,              D3D12_BLEND_OP_MIN },
        { BlendOperation::Max,              D3D12_BLEND_OP_MAX }
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // ColourMaskMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct ColourMaskMapping
    {
    public:
        ColourMask Mask;

        D3D12_COLOR_WRITE_ENABLE WriteEnable;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ColourMaskMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_ColourMaskMapping = std::to_array<ColourMaskMapping>({
        // Mask                         WriteEnable
        { ColourMask::None,             D3D12_COLOR_WRITE_ENABLE_RED }, // Default
        { ColourMask::Red,              D3D12_COLOR_WRITE_ENABLE_RED },
        { ColourMask::Green,            D3D12_COLOR_WRITE_ENABLE_GREEN },
        { ColourMask::Blue,             D3D12_COLOR_WRITE_ENABLE_BLUE },
        { ColourMask::Alpha,            D3D12_COLOR_WRITE_ENABLE_ALPHA }
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // StencilOperationMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct StencilOperationMapping
    {
    public:
        StencilOperation Operation;

        D3D12_STENCIL_OP StencilOp;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // StencilOperationMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_StencilOperationMapping = std::to_array<StencilOperationMapping>({
        // Operation                            StencilOp
        { StencilOperation::Keep,               D3D12_STENCIL_OP_KEEP },
        { StencilOperation::Zero,               D3D12_STENCIL_OP_ZERO },
        { StencilOperation::Replace,            D3D12_STENCIL_OP_REPLACE },
        { StencilOperation::IncrementAndClamp,  D3D12_STENCIL_OP_INCR_SAT },
        { StencilOperation::DecrementAndClamp,  D3D12_STENCIL_OP_DECR_SAT },
        { StencilOperation::Invert,             D3D12_STENCIL_OP_INVERT },
        { StencilOperation::IncrementAndWrap,   D3D12_STENCIL_OP_INCR },
        { StencilOperation::DecrementAndWrap,   D3D12_STENCIL_OP_DECR }
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // ComparisonFuncMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct ComparisonFuncMapping
    {
    public:
        ComparisonFunc Func;

        D3D12_COMPARISON_FUNC D3D12ComparisonFunc;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ComparisonFuncMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_ComparisonFuncMapping = std::to_array<ComparisonFuncMapping>({
        // Func                             D3D12ComparisonFunc
        { ComparisonFunc::Never,            D3D12_COMPARISON_FUNC_NEVER },
        { ComparisonFunc::Less,             D3D12_COMPARISON_FUNC_LESS },
        { ComparisonFunc::Equal,            D3D12_COMPARISON_FUNC_EQUAL },
        { ComparisonFunc::LessOrEqual,      D3D12_COMPARISON_FUNC_LESS_EQUAL },
        { ComparisonFunc::Greater,          D3D12_COMPARISON_FUNC_GREATER },
        { ComparisonFunc::NotEqual,         D3D12_COMPARISON_FUNC_NOT_EQUAL },
        { ComparisonFunc::GreaterOrEqual,   D3D12_COMPARISON_FUNC_GREATER_EQUAL },
        { ComparisonFunc::Always,           D3D12_COMPARISON_FUNC_ALWAYS }
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // RasterFillModeMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct RasterFillModeMapping
    {
    public:
        RasterFillMode Mode;

        D3D12_FILL_MODE D3D12FillMode;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // RasterFillModeMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_RasterFillModeMapping = std::to_array<RasterFillModeMapping>({
        // Mode                         D3D12FillMode
        { RasterFillMode::Solid,        D3D12_FILL_MODE_SOLID },
        { RasterFillMode::Wireframe,    D3D12_FILL_MODE_WIREFRAME },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // RasterCullingModeMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct RasterCullingModeMapping
    {
    public:
        RasterCullingMode Mode;

        D3D12_CULL_MODE D3D12CullMode;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // RasterCullingModeMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_RasterCullingModeMapping = std::to_array<RasterCullingModeMapping>({
        // Mode                     D3D12CullMode
        { RasterCullingMode::None,  D3D12_CULL_MODE_NONE },
        { RasterCullingMode::Back,  D3D12_CULL_MODE_BACK },
        { RasterCullingMode::Front, D3D12_CULL_MODE_FRONT },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // LoadOperationMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct LoadOperationMapping
    {
    public:
        LoadOperation Operation;

        D3D12_RENDER_PASS_BEGINNIOB_ACCESS_TYPE BeginningAccess;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // LoadOperationMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_LoadOperationMapping = std::to_array<LoadOperationMapping>({
        // Operation                        BeginningAccess
        { LoadOperation::None/*/DontCare*/, D3D12_RENDER_PASS_BEGINNIOB_ACCESS_TYPE_DISCARD },
        { LoadOperation::Clear,             D3D12_RENDER_PASS_BEGINNIOB_ACCESS_TYPE_CLEAR },
        { LoadOperation::Load,              D3D12_RENDER_PASS_BEGINNIOB_ACCESS_TYPE_PRESERVE }
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // StoreOperationMapping
    ////////////////////////////////////////////////////////////////////////////////////
    struct StoreOperationMapping
    {
    public:
        StoreOperation Operation;

        D3D12_RENDER_PASS_ENDIOB_ACCESS_TYPE EndingAccess;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // StoreOperationMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_StoreOperationMapping = std::to_array<StoreOperationMapping>({
        // Operation                            EndingAccess
        { StoreOperation::None/*/DontCare*/,    D3D12_RENDER_PASS_ENDIOB_ACCESS_TYPE_DISCARD},
        { StoreOperation::Store,                D3D12_RENDER_PASS_ENDIOB_ACCESS_TYPE_PRESERVE },
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // Helper methods
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const FormatMapping& FormatToFormatMapping(Format format) { OB_ASSERT((static_cast<size_t>(format) < g_FormatMapping.size()), "Format value exceeds mappings."); return g_FormatMapping[static_cast<size_t>(format)]; }

    inline constexpr D3D12_RESOURCE_STATES ResourceStateToD3D12ResourceStates(ResourceState state)
    {
        D3D12_RESOURCE_STATES result = D3D12_RESOURCE_STATE_COMMON;
        std::underlying_type_t<ResourceState> value = std::to_underlying(state);

        while (value)
        {
            int index = std::countr_zero(value);
            value &= ~(1u << index); // clear bit

            result |= g_ResourceStateMapping[static_cast<size_t>(index) + 1].D3D12States;
        }

        return result;
    }

    inline constexpr D3D12_RESOURCE_DIMENSION ImageDimensionToD3D12ResourceDimension(ImageDimension dimension) { OB_ASSERT((static_cast<size_t>(dimension) < g_ImageDimensionMapping.size()), "Dimension value exceeds mappings."); return g_ImageDimensionMapping[static_cast<size_t>(dimension)].D3D12Dimension; }
    inline constexpr D3D12_FILTER_TYPE FilterModeToD3D12FilterType(FilterMode mode) { OB_ASSERT((static_cast<size_t>(mode) < g_FilterModeMapping.size()), "Mode value exceeds mappings."); return g_FilterModeMapping[static_cast<size_t>(mode)].D3D12FilterType; }
    inline constexpr D3D12_TEXTURE_ADDRESS_MODE SamplerAddresModeToD3D12TextureAddressMode(SamplerAddressMode mode) { OB_ASSERT((static_cast<size_t>(mode) < g_SamplerAddressModeMapping.size()), "Mode value exceeds mappings."); return g_SamplerAddressModeMapping[static_cast<size_t>(mode)].D3D12TextureAddressMode; }
    inline constexpr D3D12_FILTER_REDUCTION_TYPE SamplerReductionTypeToD3D12FilterReductionType(SamplerReductionType type) { OB_ASSERT((static_cast<size_t>(type) < g_SamplerReductionTypeMapping.size()), "Type value exceeds mappings."); return g_SamplerReductionTypeMapping[static_cast<size_t>(type)].D3D12ReductionType; }

    inline constexpr DXGI_COLOR_SPACE_TYPE ColourSpaceToD3D12ColourSpace(ColourSpace space) { OB_ASSERT((static_cast<size_t>(space) < g_ColourSpaceMapping.size()), "Space value exceeds mappings."); return g_ColourSpaceMapping[static_cast<size_t>(space)].SpaceType; }

    inline constexpr D3D12_SHADER_VISIBILITY ShaderStageToD3D12ShaderVisibility(ShaderStage stage)
    {
        std::underlying_type_t<ShaderStage> value = std::to_underlying(stage);

        int index = std::countr_zero(value);
        value &= ~(1u << index); // clear bit

        if (value) // Note: If after the cleared bit there is still a stage, set it to all stages, since dx12 shader stages aren't bit flags
            return D3D12_SHADER_VISIBILITY_ALL;

        return g_ShaderStageMapping[static_cast<size_t>((std::to_underlying(stage) ? (std::countr_zero(std::to_underlying(stage)) + 1) : 0))].Visibility;
    }

    inline constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTypeToD3D12PrimitiveTopology(PrimitiveType type) { OB_ASSERT((static_cast<size_t>(type) < g_PrimitiveTypeToD3D12TypeMapping.size()), "Type value exceeds mappings."); return g_PrimitiveTypeToD3D12TypeMapping[static_cast<size_t>(type)].D3D12Type; }
    inline constexpr D3D_PRIMITIVE_TOPOLOGY PrimitiveTypeToD3DPrimitiveTopology(PrimitiveType type, uint8_t patchCount) { OB_ASSERT(((static_cast<size_t>(type) + (patchCount - 1)) < g_PrimitiveTypeToD3DTypeMapping.size()), "Type value exceeds mappings."); return g_PrimitiveTypeToD3DTypeMapping[(static_cast<size_t>(type) + (patchCount - 1))].D3DType; }
    
    inline constexpr D3D12_BLEND BlendFactorToD3D12Blend(BlendFactor factor) { OB_ASSERT((static_cast<size_t>(factor) < g_BlendFactorMapping.size()), "Factor value exceeds mappings."); return g_BlendFactorMapping[static_cast<size_t>(factor)].Blend; }
    inline constexpr D3D12_BLEND_OP BlendOperationToD3D12BlendOp(BlendOperation operation) { OB_ASSERT((static_cast<size_t>(operation) < g_BlendOperationMapping.size()), "Operation value exceeds mappings."); return g_BlendOperationMapping[static_cast<size_t>(operation)].BlendOp; }

    inline constexpr D3D12_COLOR_WRITE_ENABLE ColourMaskToD3D12ColourWriteEnable(ColourMask mask)
    {
        std::underlying_type_t<D3D12_COLOR_WRITE_ENABLE> result = 0;
        std::underlying_type_t<ColourMask> value = std::to_underlying(mask);

        while (value)
        {
            int index = std::countr_zero(value);
            value &= ~(1u << index); // clear bit

            result |= g_ColourMaskMapping[static_cast<size_t>(index) + 1].WriteEnable;
        }

        return static_cast<D3D12_COLOR_WRITE_ENABLE>(result);
    }

    inline constexpr D3D12_STENCIL_OP StencilOperationToD3D12StencilOp(StencilOperation operation) { OB_ASSERT((static_cast<size_t>(operation) < g_StencilOperationMapping.size()), "Operation value exceeds mappings."); return g_StencilOperationMapping[static_cast<size_t>(operation)].StencilOp; }
    inline constexpr D3D12_COMPARISON_FUNC ComparisonFuncToD3D12ComparisonFunc(ComparisonFunc func) { OB_ASSERT((static_cast<size_t>(func) < g_ComparisonFuncMapping.size()), "Func value exceeds mappings."); return g_ComparisonFuncMapping[static_cast<size_t>(func)].D3D12ComparisonFunc; }

    inline constexpr D3D12_FILL_MODE RasterFillModeTOD3D12FillMode(RasterFillMode mode) { OB_ASSERT((static_cast<size_t>(mode) < g_RasterFillModeMapping.size()), "Mode value exceeds mappings."); return g_RasterFillModeMapping[static_cast<size_t>(mode)].D3D12FillMode; }
    inline constexpr D3D12_CULL_MODE RasterCullingModeTOD3D12CullMode(RasterCullingMode mode) { OB_ASSERT((static_cast<size_t>(mode) < g_RasterCullingModeMapping.size()), "Mode value exceeds mappings."); return g_RasterCullingModeMapping[static_cast<size_t>(mode)].D3D12CullMode; }

    inline constexpr D3D12_RENDER_PASS_BEGINNIOB_ACCESS_TYPE LoadOperationToD3D12BeginningAccess(LoadOperation operation) { OB_ASSERT((static_cast<size_t>(operation) < g_LoadOperationMapping.size()), "Operation value exceeds mappings."); return g_LoadOperationMapping[static_cast<size_t>(operation)].BeginningAccess; }
    inline constexpr D3D12_RENDER_PASS_ENDIOB_ACCESS_TYPE StoreOperationToD3D12EndingAccess(StoreOperation operation) { OB_ASSERT((static_cast<size_t>(operation) < g_StoreOperationMapping.size()), "Operation value exceeds mappings."); return g_StoreOperationMapping[static_cast<size_t>(operation)].EndingAccess; }

    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12Resources
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12Resources
    {
    public:
        inline constexpr static uint32_t s_SRVAndUAVAndCBVStartSize = 1024 * 5; // Shader visible
        inline constexpr static uint32_t s_SamplerStartSize = 1024;             // Shader visible
        inline constexpr static uint32_t s_DSVStartSize = 16;
        inline constexpr static uint32_t s_RTVStartSize = 16;

        inline constexpr static size_t s_MaxDescriptorsPerSet = BindingLayoutSpecification::MaxBindings;
    public:
        // Constructor & Destructor
        Dx12Resources(const Device& device);
        ~Dx12Resources();

        // (Internal) Getters
        inline Dx12ManagedDescriptorHeap& GetSRVAndUAVAndCBVHeap() const { return m_SRVAndUAVAndCBVHeap; }
        inline Dx12ManagedDescriptorHeap& GetSamplerHeap() const { return m_SamplerHeap; }
        inline Dx12DynamicDescriptorHeap& GetDSVHeap() const { return m_DSVHeap; }
        inline Dx12DynamicDescriptorHeap& GetRTVHeap() const { return m_RTVHeap; }

    private:
        const Dx12Device& m_Device;

        mutable Dx12ManagedDescriptorHeap m_SRVAndUAVAndCBVHeap;    // Shader visible
        mutable Dx12ManagedDescriptorHeap m_SamplerHeap;            // Shader visible
        mutable Dx12DynamicDescriptorHeap m_DSVHeap;
        mutable Dx12DynamicDescriptorHeap m_RTVHeap;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Other
    ////////////////////////////////////////////////////////////////////////////////////
    uint8_t Dx12FormatToPlaneCount(const Device& device, DXGI_FORMAT format); // Note: 255 means format not supported.

    // helper function for texture subresource calculations
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn705766(v=vs.85).aspx
    inline constexpr uint32_t CalculateSubresource(uint32_t MipSlice, uint32_t ArraySlice, uint32_t PlaneSlice, uint32_t MipLevels, uint32_t ArraySize)
    {
        return MipSlice + (ArraySlice * MipLevels) + (PlaneSlice * MipLevels * ArraySize);
    }
#endif

}