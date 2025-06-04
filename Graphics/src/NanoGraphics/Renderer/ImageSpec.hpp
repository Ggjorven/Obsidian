#pragma once

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"

#include <cstdint>
#include <string>

namespace Nano::Graphics
{

    class Image;

    ////////////////////////////////////////////////////////////////////////////////////
    // Flags
    ////////////////////////////////////////////////////////////////////////////////////
    enum class Format : uint8_t // TODO: Remove unsupported formats
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

    enum class ImageViewType : uint8_t
    {
        Unknown = 0,

        View1D,
        View1DArray,
        View2D,
        View2DArray,
        View3D,
        ViewCube,
        ViewCubeArray
    };

    using MipLevel = uint32_t;
    using ArraySlice = uint32_t;

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

        uint32_t Width = 0, Height = 0, Depth = 0;
        uint32_t ArraySize = 1;
        uint32_t MipLevels = 1;
        uint32_t SampleCount = 1;

        bool IsShaderResource = false;
        bool IsRenderTarget = false;
        bool UseMipmaps = false;

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
        inline constexpr ImageSpecification& SetMipLevels(uint32_t count) { SampleCount = count; return *this; }
        inline constexpr ImageSpecification& SetIsShaderResource(bool enabled) { IsShaderResource = enabled; return *this; }
        inline constexpr ImageSpecification& SetIsRenderTarget(bool enabled) { IsRenderTarget = enabled; return *this; }
        inline constexpr ImageSpecification& SetUseMipmaps(bool enabled) { UseMipmaps = enabled; return *this; }
        inline constexpr ImageSpecification& SetResourceState(ResourceState state) { State = state; return *this; }
        inline constexpr ImageSpecification& SetKeepResourceState(bool enabled) { KeepResourceState = enabled; return *this; }
        inline ImageSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }
    };

    // TODO: Image Slice (in Image.hpp)

    ////////////////////////////////////////////////////////////////////////////////////
    // ImageViewSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct ImageViewSpecification
    {
    public:
        Image* ImageReference = nullptr;
        ImageViewType ViewType = ImageViewType::Unknown;

        MipLevel BaseMipLevel = 0;
        MipLevel NumMipLevels = 1;
        ArraySlice BaseArraySlice = 0;
        ArraySlice NumArraySlices = 1;

        std::string DebugName = {};

    public:
        inline constexpr ImageViewSpecification& SetImageReference(Image& image) { ImageReference = &image; return *this; }
        inline constexpr ImageViewSpecification& SetBaseMipLevel(MipLevel level) { BaseMipLevel = level; return *this; }
        inline constexpr ImageViewSpecification& SetNumMipLevels(MipLevel level) { NumMipLevels = level; return *this; }
        inline constexpr ImageViewSpecification& SetBaseArraySlice(ArraySlice base) { BaseArraySlice = base; return *this; }
        inline constexpr ImageViewSpecification& SetNumArraySlices(ArraySlice num) { NumArraySlices = num; return *this; }
        inline ImageViewSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // SamplerSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct SamplerSpecification
    {
    public:
        Maths::Vec4<float> BorderColour = { 1.0f, 1.0f, 1.0f, 1.0f };
        float MaxAnisotropy = 1.0f;
        float MipBias = 0.0f;

        bool MinFilter = true;
        bool MagFilter = true;
        bool MipFilter = true;
        
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
        inline constexpr SamplerSpecification& SetMinFilter(bool enabled) { MinFilter = enabled; return *this; }
        inline constexpr SamplerSpecification& SetMagFilter(bool enabled) { MagFilter = enabled; return *this; }
        inline constexpr SamplerSpecification& SetMipFilter(bool enabled) { MipFilter = enabled; return *this; }
        inline constexpr SamplerSpecification& SetAddressModeU(SamplerAddressMode u) { AddressU = u; return *this; }
        inline constexpr SamplerSpecification& SetAddressModeV(SamplerAddressMode v) { AddressV = v; return *this; }
        inline constexpr SamplerSpecification& SetAddressModeW(SamplerAddressMode w) { AddressW = w; return *this; }
        inline constexpr SamplerSpecification& SetUVW(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w) { AddressU = u; AddressV = v; AddressW = w; return *this; }
        inline constexpr SamplerSpecification& SetReductionType(SamplerReductionType type) { ReductionType = type; return *this; }
        inline SamplerSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }

    };

}