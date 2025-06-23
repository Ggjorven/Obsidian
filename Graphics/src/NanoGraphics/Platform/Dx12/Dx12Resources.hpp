#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"

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
	// Dx12FormatMapping
	////////////////////////////////////////////////////////////////////////////////////
    struct Dx12FormatMapping
    {
    public:
        Format AbstractFormat;
        
        DXGI_FORMAT ResourceFormat;
        DXGI_FORMAT SRVFormat;
        DXGI_FORMAT RTVFormat;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12FormatMapping array
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const auto g_FormatMappings = std::to_array<Dx12FormatMapping>({
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
    // Helper methods
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr const Dx12FormatMapping& FormatToDx12FormatMapping(Format format) { NG_ASSERT((static_cast<size_t>(format) < g_FormatMappings.size()), "Format value exceeds mappings."); return g_FormatMappings[static_cast<size_t>(format)]; }

    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12Resources
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12Resources
    {
    public:
        inline constexpr static uint32_t s_SRVAndUAVStartSize = 16;
        inline constexpr static uint32_t s_SamplerStartSize = 16;
        inline constexpr static uint32_t s_DSVStartSize = 16;
        inline constexpr static uint32_t s_RTVStartSize = 16;
    public:
        struct Heap
        {
        public:
            struct Entry
            {
            public:
                uint32_t Amount = 0;
                CD3DX12_CPU_DESCRIPTOR_HANDLE Handle = {};
            };
        public: // TODO: Make private?
            uint32_t MaxSize; // Amount of descriptors allocateable
            uint32_t Count = 0; // Amount of descriptors allocated

            uint32_t DescriptorSize = 0; // Size of a descriptor
            CD3DX12_CPU_DESCRIPTOR_HANDLE Offset = {}; // Current offset into heap

            std::vector<Entry> FreeEntries = {}; // Free'd up descriptors that can be reused.
            
            ID3D12DescriptorHeap* DescriptorHeap = nullptr;
            D3D12_DESCRIPTOR_HEAP_TYPE Type;
            bool IsShaderVisible;

        public:
            // Constructor & Destructor
            Heap(const Device& device, uint32_t maxSize, D3D12_DESCRIPTOR_HEAP_TYPE type, bool isShaderVisible);
            ~Heap();

            // Methods
            CD3DX12_CPU_DESCRIPTOR_HANDLE CreateSRV(Format format, ImageDimension dimension, const ImageSubresourceSpecification& subresources, const ImageSpecification& specs, ID3D12Resource* resource);
            CD3DX12_CPU_DESCRIPTOR_HANDLE CreateUAV(Format format, ImageDimension dimension, const ImageSubresourceSpecification& subresources, const ImageSpecification& specs, ID3D12Resource* resource);
            CD3DX12_CPU_DESCRIPTOR_HANDLE CreateRTV(Format format, const ImageSubresourceSpecification& subresources, const ImageSpecification& specs, ID3D12Resource* resource);
            CD3DX12_CPU_DESCRIPTOR_HANDLE CreateDSV(const ImageSubresourceSpecification& subresources, const ImageSpecification& specs, ID3D12Resource* resource, bool isReadOnly = false);
            void Free(CD3DX12_CPU_DESCRIPTOR_HANDLE handle);

            void Grow(uint32_t minNewSize);

        private:
            // Private methods
            CD3DX12_CPU_DESCRIPTOR_HANDLE GetNextHandle();

        private:
            const Dx12Device& m_Device;
        };
    public:
        // Constructor & Destructor
        Dx12Resources(const Device& device);
        ~Dx12Resources();

        // (Internal) Getters
        inline Heap& GetSRVAndUAVHeap() const { return m_SRVAndUAVHeap; }
        inline Heap& GetSamplerHeap() const { return m_SamplerHeap; }
        inline Heap& GetDSVHeap() const { return m_DSVHeap; }
        inline Heap& GetRTVHeap() const { return m_RTVHeap; }

    private:
        const Dx12Device& m_Device;

        mutable Heap m_SRVAndUAVHeap;
        mutable Heap m_SamplerHeap;
        mutable Heap m_DSVHeap;
        mutable Heap m_RTVHeap;
    };
#endif

}