#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"
#include "NanoGraphics/Renderer/BindingsSpec.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Descriptors.hpp"

#include <vector>

namespace Nano::Graphics
{
    class Device;
}

namespace Nano::Graphics::Internal
{

    class Dx12Device;

#if defined(NG_API_DX12)
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
        { ResourceState::Unknown,           D3D12_RESOURCE_STATE_COMMON },
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
        { ResourceState::StreamOut,         D3D12_RESOURCE_STATE_STREAM_OUT },
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
        { ShaderStage::AllGraphics,     D3D12_SHADER_VISIBILITY_ALL }, // Placeholder

        // Note: Other shaderstages are not supported
    });

    ////////////////////////////////////////////////////////////////////////////////////
    // Helper methods
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const FormatMapping& FormatToFormatMapping(Format format) { NG_ASSERT((static_cast<size_t>(format) < g_FormatMapping.size()), "Format value exceeds mappings."); return g_FormatMapping[static_cast<size_t>(format)]; }

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

    inline constexpr D3D12_RESOURCE_DIMENSION ImageDimensionToD3D12ResourceDimension(ImageDimension dimension) { NG_ASSERT((static_cast<size_t>(dimension) < g_ImageDimensionMapping.size()), "Dimension value exceeds mappings."); return g_ImageDimensionMapping[static_cast<size_t>(dimension)].D3D12Dimension; }
    inline constexpr D3D12_FILTER_TYPE FilterModeToD3D12FilterType(FilterMode mode) { NG_ASSERT((static_cast<size_t>(mode) < g_FilterModeMapping.size()), "Mode value exceeds mappings."); return g_FilterModeMapping[static_cast<size_t>(mode)].D3D12FilterType; }
    inline constexpr D3D12_TEXTURE_ADDRESS_MODE SamplerAddresModeToD3D12TextureAddressMode(SamplerAddressMode mode) { NG_ASSERT((static_cast<size_t>(mode) < g_SamplerAddressModeMapping.size()), "Mode value exceeds mappings."); return g_SamplerAddressModeMapping[static_cast<size_t>(mode)].D3D12TextureAddressMode; }
    inline constexpr D3D12_FILTER_REDUCTION_TYPE SamplerReductionTypeToD3D12FilterReductionType(SamplerReductionType type) { NG_ASSERT((static_cast<size_t>(type) < g_SamplerReductionTypeMapping.size()), "Type value exceeds mappings."); return g_SamplerReductionTypeMapping[static_cast<size_t>(type)].D3D12ReductionType; }

    inline constexpr D3D12_SHADER_VISIBILITY ShaderStageToD3D12ShaderVisibility(ShaderStage stage) // Note: We take the first stage, since D3D12 visibility are not bit flags
    {
        return g_ShaderStageMapping[static_cast<size_t>((std::to_underlying(stage) ? (std::countr_zero(std::to_underlying(stage)) + 1) : 0))].Visibility;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12Resources
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12Resources
    {
    public:
        inline constexpr static uint32_t s_SRVAndUAVStartSize = 1024;               // Shader visible
        inline constexpr static uint32_t s_SamplerStartSize = s_SRVAndUAVStartSize; // Shader visible
        inline constexpr static uint32_t s_DSVStartSize = 16;
        inline constexpr static uint32_t s_RTVStartSize = 16;

        inline constexpr static size_t s_MaxDescriptorsPerSet = BindingLayoutSpecification::MaxBindings;
    public:
        // Constructor & Destructor
        Dx12Resources(const Device& device);
        ~Dx12Resources();

        // (Internal) Getters
        inline Dx12ManagedDescriptorHeap& GetSRVAndUAVHeap() const { return m_SRVAndUAVHeap; }
        inline Dx12ManagedDescriptorHeap& GetSamplerHeap() const { return m_SamplerHeap; }
        inline Dx12DynamicDescriptorHeap& GetDSVHeap() const { return m_DSVHeap; }
        inline Dx12DynamicDescriptorHeap& GetRTVHeap() const { return m_RTVHeap; }

    private:
        const Dx12Device& m_Device;

        mutable Dx12ManagedDescriptorHeap m_SRVAndUAVHeap;  // Shader visible
        mutable Dx12ManagedDescriptorHeap m_SamplerHeap;    // Shader visible
        mutable Dx12DynamicDescriptorHeap m_DSVHeap;
        mutable Dx12DynamicDescriptorHeap m_RTVHeap;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Other
    ////////////////////////////////////////////////////////////////////////////////////
    // helper function for texture subresource calculations
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn705766(v=vs.85).aspx
    inline constexpr uint32_t CalculateSubresource(uint32_t MipSlice, uint32_t ArraySlice, uint32_t PlaneSlice, uint32_t MipLevels, uint32_t ArraySize)
    {
        return MipSlice + (ArraySlice * MipLevels) + (PlaneSlice * MipLevels * ArraySize);
    }
#endif

}