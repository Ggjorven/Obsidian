#pragma once

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"

#include <cstdint>
#include <cmath>
#include <string>
#include <limits>
#include <numeric>

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

        Count,
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

        bool IsShaderResource = false;
        bool IsUnorderedAccessed = false;
        bool IsRenderTarget = false;

        ResourceState State = ResourceState::Unknown;
        bool KeepResourceState = false; // Note: After executing commands will go back to set ResourceState ^

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
        inline ImageSpecification& SetMaxMipLevels() { MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(Width, Height)))) + 1; return *this; } // Note: Will auto set miplevels
        inline constexpr ImageSpecification& SetSampleCount(uint32_t count) { SampleCount = count; return *this; }
        inline constexpr ImageSpecification& SetIsShaderResource(bool enabled) { IsShaderResource = enabled; return *this; }
        inline constexpr ImageSpecification& SetIsRenderTarget(bool enabled) { IsRenderTarget = enabled; return *this; }
        inline constexpr ImageSpecification& SetResourceState(ResourceState state) { State = state; return *this; }
        inline constexpr ImageSpecification& SetKeepResourceState(bool enabled) { KeepResourceState = enabled; return *this; }
        inline ImageSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }
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
        MipLevel NumMipLevels = 1;
        ArraySlice BaseArraySlice = 0;
        ArraySlice NumArraySlices = 1;

    public:
        // Setters
        inline constexpr ImageSubresourceSpecification& SetBaseMipLevel(MipLevel level) { BaseMipLevel = level; return *this; }
        inline constexpr ImageSubresourceSpecification& SetNumMipLevels(MipLevel level) { NumMipLevels = level; return *this; }
        inline constexpr ImageSubresourceSpecification& SetBaseArraySlice(ArraySlice base) { BaseArraySlice = base; return *this; }
        inline constexpr ImageSubresourceSpecification& SetNumArraySlices(ArraySlice num) { NumArraySlices = num; return *this; }

        // Operators
        inline constexpr bool operator == (const ImageSubresourceSpecification& other) const { return ((BaseMipLevel == other.BaseMipLevel) && (NumMipLevels == other.NumMipLevels) && (BaseArraySlice == other.BaseArraySlice) && (NumArraySlices == other.NumArraySlices)); }
        inline constexpr bool operator != (const ImageSubresourceSpecification& other) const { return !(*this == other); }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ImageSlice
    ////////////////////////////////////////////////////////////////////////////////////
    struct ImageSliceSpecification
    {
    public:
        uint32_t X = 0;
        uint32_t Y = 0;
        uint32_t Z = 0;

        uint32_t Width = std::numeric_limits<uint32_t>::max();
        uint32_t Height = std::numeric_limits<uint32_t>::max();
        uint32_t Depth = std::numeric_limits<uint32_t>::max();

        MipLevel ImageMipLevel = 0;
        ArraySlice ImageArraySlice = 0;

    public:
        // Setters
        inline constexpr ImageSliceSpecification& setOrigin(uint32_t x, uint32_t y, uint32_t z = 0) { X = x; Y = y; Z = z; return *this; }
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
        float MaxAnisotropy = MaxMaxAnisotropyValue;
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
    inline constexpr bool FormatHasDepth(Format format)
    {
        switch (format)
        {
        case Format::D16:
        case Format::D24S8:
        case Format::D32:
        case Format::D32S8:
            return true;

        default:
            break;
        }

        return false;
    }

    inline constexpr bool FormatHasStencil(Format format)
    {
        switch (format)
        {
        case Format::D24S8:
        case Format::X24G8UInt:
        case Format::D32S8:
        case Format::X32G8UInt:
            return true;

        default:
            break;
        }

        return false;
    }

}