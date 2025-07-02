#pragma once

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"

#include <cstdint>
#include <cmath>
#include <limits>
#include <string>
#include <numeric>
#include <string>

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Flags
    ////////////////////////////////////////////////////////////////////////////////////
    enum class Format : uint8_t
    {
        Unknown = 0,

        R8UInt,
        R8SInt,
        R8Unorm,
        R8Snorm,
        RG8UInt,
        RG8SInt,
        RG8Unorm,
        RG8Snorm,
        R16UInt,
        R16SInt,
        R16Unorm,
        R16Snorm,
        R16Float,
        BGRA4Unorm,
        B5G6R5Unorm,
        B5G5R5A1Unorm,
        RGBA8UInt,
        RGBA8SInt,
        RGBA8Unorm,
        RGBA8Snorm,
        BGRA8Unorm,
        SRGBA8Unorm,
        SBGRA8Unorm,
        R10G10B10A2Unorm,
        R11G11B10Float,
        RG16UInt,
        RG16SInt,
        RG16Unorm,
        RG16Snorm,
        RG16Float,
        R32UInt,
        R32SInt,
        R32Float,
        RGBA16UInt,
        RGBA16SInt,
        RGBA16Float,
        RGBA16Unorm,
        RGBA16Snorm,
        RG32UInt,
        RG32SInt,
        RG32Float,
        RGB32UInt,
        RGB32SInt,
        RGB32Float,
        RGBA32UInt,
        RGBA32SInt,
        RGBA32Float,

        D16,
        D24S8,
        X24G8UInt,
        D32,
        D32S8,
        X32G8UInt,

        BC1Unorm,
        BC1UnormSRGB,
        BC2Unorm,
        BC2UnormSRGB,
        BC3Unorm,
        BC3UnormSRGB,
        BC4Unorm,
        BC4Snorm,
        BC5Unorm,
        BC5Snorm,
        BC6HUFloat,
        BC6HSFloat,
        BC7Unorm,
        BC7UnormSRGB,
    };

    enum class FormatKind : uint8_t
    {
        Integer,
        Normalized,
        Float,
        DepthStencil
    };

    enum class ImageDimension : uint8_t
    {
        Unknown = 0,

        Image1D,
        Image1DArray,
        Image2D,
        Image2DArray,
        ImageCube,
        ImageCubeArray,
        Image2DMS,
        Image2DMSArray,
        Image3D
    };

    enum class ImageSubresourceViewType : uint8_t
    {
        AllAspects = 0,
        DepthOnly,
        StencilOnly
    };

    enum class ImageSubresourceViewUsage : uint8_t
    {
        SRV = 0,
        UAV,
        RTV,
        DSV, 
    };

    using MipLevel = uint32_t;
    using ArraySlice = uint32_t;

    enum class FilterMode : uint8_t
    {
        None = 0,

        Nearest,
        Linear
    };

    enum class SamplerAddressMode : uint8_t
    {
        // D3D names
        Clamp,
        Wrap,
        Border,
        Mirror,
        MirrorOnce,

        // Vulkan names
        ClampToEdge = Clamp,
        Repeat = Wrap,
        ClampToBorder = Border,
        MirroredRepeat = Mirror,
        MirrorClampToEdge = MirrorOnce
    };

    enum class SamplerReductionType : uint8_t
    {
        Standard,
        Comparison,
        Minimum,
        Maximum
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ImageSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct ImageSpecification
    {
    public:
        Format ImageFormat = Format::Unknown;
        ImageDimension Dimension = ImageDimension::Unknown;
        
        uint32_t Width = 0, Height = 0, Depth = 1;
        uint32_t ArraySize = 1;
        uint32_t MipLevels = 1; // Note: Max = static_cast<uint32_t>(std::floor(std::log2(std::max(Width, Height)))) + 1;
        
        uint32_t SampleCount = 1;
        uint32_t SampleQuality = 0;

        bool IsShaderResource = false;
        bool IsUnorderedAccessed = false;
        bool IsRenderTarget = false;

        bool IsTypeless = false; // For storage?

        ResourceState PermanentState = ResourceState::Unknown; // Note: Anything other than Unknown sets it to be permanent

        std::string DebugName = {};

    public:
        // Setters
        inline constexpr ImageSpecification& SetImageFormat(Format format) { ImageFormat = format; return *this; }
        inline constexpr ImageSpecification& SetImageDimension(ImageDimension dimension) { Dimension = dimension; return *this; }
        
        inline constexpr ImageSpecification& SetWidth(uint32_t width) { Width = width; return *this; }
        inline constexpr ImageSpecification& SetHeight(uint32_t height) { Height = height; return *this; }
        inline constexpr ImageSpecification& SetDepth(uint32_t depth) { Depth = depth; return *this; }
        inline constexpr ImageSpecification& SetWidthAndHeight(uint32_t width, uint32_t height) { Width = width; Height = height; return *this; }
        inline constexpr ImageSpecification& SetWidthAndHeightAndDepth(uint32_t width, uint32_t height, uint32_t depth) { Width = width; Height = height; Depth = depth; return *this; }
        
        inline constexpr ImageSpecification& SetArraySize(uint32_t size) { ArraySize = size; return *this; }
        inline constexpr ImageSpecification& SetMipLevels(uint32_t mipLevels) { MipLevels = mipLevels; return *this; }
        inline ImageSpecification& SetMipLevelsToMax() { MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(Width, Height)))) + 1; return *this; } // Note: Will auto set miplevels
        
        inline constexpr ImageSpecification& SetSampleCount(uint32_t count) { SampleCount = count; return *this; }
        inline constexpr ImageSpecification& SetSampleQuality(uint32_t quality) { SampleQuality = quality; return *this; }
        
        inline constexpr ImageSpecification& SetIsShaderResource(bool enabled) { IsShaderResource = enabled; return *this; }
        inline constexpr ImageSpecification& SetIsUnorderedAccessed(bool enabled) { IsUnorderedAccessed = enabled; return *this; }
        inline constexpr ImageSpecification& SetIsRenderTarget(bool enabled) { IsRenderTarget = enabled; return *this; }
        
        inline constexpr ImageSpecification& SetPermanentState(ResourceState state) { PermanentState = state; return *this; }

        inline constexpr ImageSpecification& SetIsTypeless(bool enabled) { IsTypeless = enabled; return *this; }
        
        inline ImageSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }

        // Getters
        inline constexpr bool HasPermanentState() const { return (PermanentState != ResourceState::Unknown); }

        // Operators
        inline constexpr bool operator == (const ImageSpecification& other) const { return ((ImageFormat == other.ImageFormat) && (Dimension == other.Dimension) && (Width == other.Width) && (Height == other.Height) && (Depth == other.Depth) && (ArraySize == other.ArraySize) && (MipLevels == other.MipLevels) && (SampleCount == other.SampleCount) && (IsShaderResource == other.IsShaderResource) && (IsUnorderedAccessed == other.IsUnorderedAccessed) && (IsRenderTarget == other.IsRenderTarget) && (PermanentState == other.PermanentState)); }
        inline constexpr bool operator != (const ImageSpecification& other) const { return !(*this == other); }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ImageViewSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct ImageSubresourceSpecification // Note: This is used to specify a Subresource of an Image for a command.
    {
    public:
        inline constexpr static MipLevel AllMipLevels = std::numeric_limits<uint32_t>::max();
        inline constexpr static ArraySlice AllArraySlices = std::numeric_limits<uint32_t>::max();
    public:
        MipLevel BaseMipLevel = 0;
        MipLevel NumMipLevels = AllMipLevels;
        ArraySlice BaseArraySlice = 0;
        ArraySlice NumArraySlices = AllArraySlices;

    public:
        // Setters
        inline constexpr ImageSubresourceSpecification& SetBaseMipLevel(MipLevel level) { BaseMipLevel = level; return *this; }
        inline constexpr ImageSubresourceSpecification& SetNumMipLevels(MipLevel level) { NumMipLevels = level; return *this; }
        inline constexpr ImageSubresourceSpecification& SetBaseArraySlice(ArraySlice base) { BaseArraySlice = base; return *this; }
        inline constexpr ImageSubresourceSpecification& SetNumArraySlices(ArraySlice num) { NumArraySlices = num; return *this; }

        // Getters
        inline bool IsEntireTexture(const ImageSpecification& imageSpecs) const
        {
            if (BaseMipLevel > 0u || BaseMipLevel + NumMipLevels < imageSpecs.MipLevels)
                return false;

            switch (imageSpecs.Dimension)
            {
            case ImageDimension::Image1DArray:  
            case ImageDimension::Image2DArray:  
            case ImageDimension::ImageCube:     
            case ImageDimension::ImageCubeArray:
            case ImageDimension::Image2DMSArray:
            {
                if (BaseArraySlice > 0u || BaseArraySlice + NumArraySlices < imageSpecs.ArraySize)
                    return false;
                break;
            }

            default:
                break;
            }

            return true;
        }

        inline static size_t SubresourceIndex(MipLevel mipLevel, ArraySlice arraySlice, const ImageSpecification& specs)
        {
            return static_cast<size_t>(mipLevel) + arraySlice * static_cast<size_t>(specs.MipLevels);
        }

        // Operators
        inline constexpr bool operator == (const ImageSubresourceSpecification& other) const 
        { 
            return ((BaseMipLevel == other.BaseMipLevel) && (NumMipLevels == other.NumMipLevels) && (BaseArraySlice == other.BaseArraySlice) && (NumArraySlices == other.NumArraySlices)); 
        }
        inline constexpr bool operator != (const ImageSubresourceSpecification& other) const { return !(*this == other); }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ImageSlice
    ////////////////////////////////////////////////////////////////////////////////////
    struct ImageSliceSpecification
    {
    public:
        // Note: std::numeric_limits<uint32_t>::max() is equal to the full image.
        inline constexpr static uint32_t FullSize = std::numeric_limits<uint32_t>::max();
    public:
        int32_t X = 0;
        int32_t Y = 0;
        int32_t Z = 0;

        uint32_t Width = FullSize;
        uint32_t Height = FullSize;
        uint32_t Depth = FullSize;

        MipLevel ImageMipLevel = 0;
        ArraySlice ImageArraySlice = 0;

    public:
        // Setters
        inline constexpr ImageSliceSpecification& SetOffset(int32_t x, int32_t y, int32_t z = 0) { X = x; Y = y; Z = z; return *this; }
        inline constexpr ImageSliceSpecification& setWidth(uint32_t width) { Width = width; return *this; }
        inline constexpr ImageSliceSpecification& setHeight(uint32_t height) { Height = height; return *this; }
        inline constexpr ImageSliceSpecification& setDepth(uint32_t depth) { Depth = depth; return *this; }
        inline constexpr ImageSliceSpecification& SetWidthAndHeight(uint32_t width, uint32_t height) { Width = width; Height = height; return *this; }
        inline constexpr ImageSliceSpecification& SetWidthAndHeightAndDepth(uint32_t width, uint32_t height, uint32_t depth) { Width = width; Height = height; Depth = depth; return *this; }
        inline constexpr ImageSliceSpecification& SetMipLevel(MipLevel level) { ImageMipLevel = level; return *this; }
        inline constexpr ImageSliceSpecification& SetArraySlice(ArraySlice slice) { ImageArraySlice = slice; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // SamplerSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct SamplerSpecification
    {
    public:
        inline constexpr static float DisableMaxAnisotropyValue = 1.0f;
        inline constexpr static float MaxMaxAnisotropyValue = -1.0f;
    public:
        Maths::Vec4<float> BorderColour = { 1.0f, 1.0f, 1.0f, 1.0f };
        float MaxAnisotropy = DisableMaxAnisotropyValue;
        float MipBias = 0.0f;

        FilterMode MinFilter = FilterMode::Nearest;
        FilterMode MagFilter = FilterMode::Nearest;
        FilterMode MipFilter = FilterMode::Nearest;
        
        SamplerAddressMode AddressU = SamplerAddressMode::Clamp;
        SamplerAddressMode AddressV = SamplerAddressMode::Clamp;
        SamplerAddressMode AddressW = SamplerAddressMode::Clamp;

        SamplerReductionType ReductionType = SamplerReductionType::Standard;

        std::string DebugName = {};

    public:
        // Setters
        inline constexpr SamplerSpecification& SetBorderColour(const Maths::Vec4<float> colour) { BorderColour = colour; return *this; }
        inline constexpr SamplerSpecification& SetMaxAnisotropy(float max) { MaxAnisotropy = max; return *this; }
        inline constexpr SamplerSpecification& SetMipBias(float bias) { MipBias = bias; return *this; }
        inline constexpr SamplerSpecification& SetMinFilter(FilterMode filter) { MinFilter = filter; return *this; }
        inline constexpr SamplerSpecification& SetMagFilter(FilterMode filter) { MagFilter = filter; return *this; }
        inline constexpr SamplerSpecification& SetMipFilter(FilterMode filter) { MipFilter = filter; return *this; }
        inline constexpr SamplerSpecification& SetAddressModeU(SamplerAddressMode u) { AddressU = u; return *this; }
        inline constexpr SamplerSpecification& SetAddressModeV(SamplerAddressMode v) { AddressV = v; return *this; }
        inline constexpr SamplerSpecification& SetAddressModeW(SamplerAddressMode w) { AddressW = w; return *this; }
        inline constexpr SamplerSpecification& SetUVW(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w) { AddressU = u; AddressV = v; AddressW = w; return *this; }
        inline constexpr SamplerSpecification& SetReductionType(SamplerReductionType type) { ReductionType = type; return *this; }
        inline SamplerSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }

    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Helper
    ////////////////////////////////////////////////////////////////////////////////////
    namespace Internal
    {

        ////////////////////////////////////////////////////////////////////////////////////
        // FormatInfo
        ////////////////////////////////////////////////////////////////////////////////////
        struct FormatInfo
        {
        public:
            Format FormatType;
            uint8_t BytesPerBlock;
            uint8_t BlockSize;
            FormatKind Kind;

            // Map all booleans to 1 byte (1 bit per bool)
            bool HasRed     : 1;
            bool HasGreen   : 1;
            bool HasBlue    : 1;
            bool HasAlpha   : 1;
            bool HasDepth   : 1;
            bool HasStencil : 1;
            bool IsSigned   : 1;
            bool IsSRGB     : 1;
        };

        ////////////////////////////////////////////////////////////////////////////////////
        // FormatInfo array
        ////////////////////////////////////////////////////////////////////////////////////
        inline constexpr const auto g_FormatInfo = std::to_array<FormatInfo>({
            // Format                   Bytes  Block    Kind                      Red    Green  Blue   Alpha  Depth  Stencl Signed SRGB
            { Format::Unknown,          0,     0,       FormatKind::Integer,      false, false, false, false, false, false, false, false },
            { Format::R8UInt,           1,     1,       FormatKind::Integer,      true,  false, false, false, false, false, false, false },
            { Format::R8SInt,           1,     1,       FormatKind::Integer,      true,  false, false, false, false, false, true,  false },
            { Format::R8Unorm,          1,     1,       FormatKind::Normalized,   true,  false, false, false, false, false, false, false },
            { Format::R8Snorm,          1,     1,       FormatKind::Normalized,   true,  false, false, false, false, false, true,  false },
            { Format::RG8UInt,          2,     1,       FormatKind::Integer,      true,  true,  false, false, false, false, false, false },
            { Format::RG8SInt,          2,     1,       FormatKind::Integer,      true,  true,  false, false, false, false, true,  false },
            { Format::RG8Unorm,         2,     1,       FormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
            { Format::RG8Snorm,         2,     1,       FormatKind::Normalized,   true,  true,  false, false, false, false, true,  false },
            { Format::R16UInt,          2,     1,       FormatKind::Integer,      true,  false, false, false, false, false, false, false },
            { Format::R16SInt,          2,     1,       FormatKind::Integer,      true,  false, false, false, false, false, true,  false },
            { Format::R16Unorm,         2,     1,       FormatKind::Normalized,   true,  false, false, false, false, false, false, false },
            { Format::R16Snorm,         2,     1,       FormatKind::Normalized,   true,  false, false, false, false, false, true,  false },
            { Format::R16Float,         2,     1,       FormatKind::Float,        true,  false, false, false, false, false, true,  false },
            { Format::BGRA4Unorm,       2,     1,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
            { Format::B5G6R5Unorm,      2,     1,       FormatKind::Normalized,   true,  true,  true,  false, false, false, false, false },
            { Format::B5G5R5A1Unorm,    2,     1,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
            { Format::RGBA8UInt,        4,     1,       FormatKind::Integer,      true,  true,  true,  true,  false, false, false, false },
            { Format::RGBA8SInt,        4,     1,       FormatKind::Integer,      true,  true,  true,  true,  false, false, true,  false },
            { Format::RGBA8Unorm,       4,     1,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
            { Format::RGBA8Snorm,       4,     1,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, true,  false },
            { Format::BGRA8Unorm,       4,     1,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
            { Format::SRGBA8Unorm,      4,     1,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
            { Format::SBGRA8Unorm,      4,     1,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
            { Format::R10G10B10A2Unorm, 4,     1,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
            { Format::R11G11B10Float,   4,     1,       FormatKind::Float,        true,  true,  true,  false, false, false, false, false },
            { Format::RG16UInt,         4,     1,       FormatKind::Integer,      true,  true,  false, false, false, false, false, false },
            { Format::RG16SInt,         4,     1,       FormatKind::Integer,      true,  true,  false, false, false, false, true,  false },
            { Format::RG16Unorm,        4,     1,       FormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
            { Format::RG16Snorm,        4,     1,       FormatKind::Normalized,   true,  true,  false, false, false, false, true,  false },
            { Format::RG16Float,        4,     1,       FormatKind::Float,        true,  true,  false, false, false, false, true,  false },
            { Format::R32UInt,          4,     1,       FormatKind::Integer,      true,  false, false, false, false, false, false, false },
            { Format::R32SInt,          4,     1,       FormatKind::Integer,      true,  false, false, false, false, false, true,  false },
            { Format::R32Float,         4,     1,       FormatKind::Float,        true,  false, false, false, false, false, true,  false },
            { Format::RGBA16UInt,       8,     1,       FormatKind::Integer,      true,  true,  true,  true,  false, false, false, false },
            { Format::RGBA16SInt,       8,     1,       FormatKind::Integer,      true,  true,  true,  true,  false, false, true,  false },
            { Format::RGBA16Float,      8,     1,       FormatKind::Float,        true,  true,  true,  true,  false, false, true,  false },
            { Format::RGBA16Unorm,      8,     1,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
            { Format::RGBA16Snorm,      8,     1,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, true,  false },
            { Format::RG32UInt,         8,     1,       FormatKind::Integer,      true,  true,  false, false, false, false, false, false },
            { Format::RG32SInt,         8,     1,       FormatKind::Integer,      true,  true,  false, false, false, false, true,  false },
            { Format::RG32Float,        8,     1,       FormatKind::Float,        true,  true,  false, false, false, false, true,  false },
            { Format::RGB32UInt,        12,    1,       FormatKind::Integer,      true,  true,  true,  false, false, false, false, false },
            { Format::RGB32SInt,        12,    1,       FormatKind::Integer,      true,  true,  true,  false, false, false, true,  false },
            { Format::RGB32Float,       12,    1,       FormatKind::Float,        true,  true,  true,  false, false, false, true,  false },
            { Format::RGBA32UInt,       16,    1,       FormatKind::Integer,      true,  true,  true,  true,  false, false, false, false },
            { Format::RGBA32SInt,       16,    1,       FormatKind::Integer,      true,  true,  true,  true,  false, false, true,  false },
            { Format::RGBA32Float,      16,    1,       FormatKind::Float,        true,  true,  true,  true,  false, false, true,  false },
            { Format::D16,              2,     1,       FormatKind::DepthStencil, false, false, false, false, true,  false, false, false },
            { Format::D24S8,            4,     1,       FormatKind::DepthStencil, false, false, false, false, true,  true,  false, false },
            { Format::X24G8UInt,        4,     1,       FormatKind::Integer,      false, false, false, false, false, true,  false, false },
            { Format::D32,              4,     1,       FormatKind::DepthStencil, false, false, false, false, true,  false, false, false },
            { Format::D32S8,            8,     1,       FormatKind::DepthStencil, false, false, false, false, true,  true,  false, false },
            { Format::X32G8UInt,        8,     1,       FormatKind::Integer,      false, false, false, false, false, true,  false, false },
            { Format::BC1Unorm,         8,     4,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
            { Format::BC1UnormSRGB,     8,     4,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
            { Format::BC2Unorm,         16,    4,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
            { Format::BC2UnormSRGB,     16,    4,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
            { Format::BC3Unorm,         16,    4,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
            { Format::BC3UnormSRGB,     16,    4,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
            { Format::BC4Unorm,         8,     4,       FormatKind::Normalized,   true,  false, false, false, false, false, false, false },
            { Format::BC4Snorm,         8,     4,       FormatKind::Normalized,   true,  false, false, false, false, false, true,  false },
            { Format::BC5Unorm,         16,    4,       FormatKind::Normalized,   true,  true,  false, false, false, false, false, false },
            { Format::BC5Snorm,         16,    4,       FormatKind::Normalized,   true,  true,  false, false, false, false, true,  false },
            { Format::BC6HUFloat,       16,    4,       FormatKind::Float,        true,  true,  true,  false, false, false, false, false },
            { Format::BC6HSFloat,       16,    4,       FormatKind::Float,        true,  true,  true,  false, false, false, true,  false },
            { Format::BC7Unorm,         16,    4,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, false },
            { Format::BC7UnormSRGB,     16,    4,       FormatKind::Normalized,   true,  true,  true,  true,  false, false, false, true  },
        });

        ////////////////////////////////////////////////////////////////////////////////////
        // Helper methods
        ////////////////////////////////////////////////////////////////////////////////////
        inline constexpr bool FormatHasDepth(Format format) { return g_FormatInfo[static_cast<size_t>(format)].HasDepth; }
        inline constexpr bool FormatHasStencil(Format format) { return g_FormatInfo[static_cast<size_t>(format)].HasStencil; }
        inline constexpr const FormatInfo& FormatToFormatInfo(Format format) { return g_FormatInfo[static_cast<size_t>(format)]; }
        
        inline constexpr ImageSubresourceViewType FormatToImageSubresourceViewType(Format format)
        {
            const FormatInfo& formatInfo = FormatToFormatInfo(format);

            if (formatInfo.HasDepth)
                return ImageSubresourceViewType::DepthOnly;
            else if (formatInfo.HasStencil)
                return ImageSubresourceViewType::StencilOnly;
            
            return ImageSubresourceViewType::AllAspects;
        }

    }

}