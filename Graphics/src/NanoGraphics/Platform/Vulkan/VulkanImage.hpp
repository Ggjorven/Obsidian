#pragma once

#include "NanoGraphics/Core/Logging.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <type_traits>

namespace Nano::Graphics
{
	class Device;
	class Image;
}

namespace Nano::Graphics::Internal
{

	class VulkanImage;
	class VulkanDevice;

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanImageSubresourceView
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanImageSubresourceView
	{
	public:
		using Key = std::tuple<ImageSubresourceSpecification, ImageSubresourceViewType, ImageDimension, Format, VkImageUsageFlags>;
		struct Hash
		{
		public:
			uint64_t operator()(const Key& key) const noexcept(true)
			{
				const auto& [specs, viewType, dimension, format, usage] = key;

				uint64_t hash = 0;

				hash = Nano::Hash::Combine(hash, std::hash<decltype(specs.BaseMipLevel)>{}(specs.BaseMipLevel));
				hash = Nano::Hash::Combine(hash, std::hash<decltype(specs.NumMipLevels)>{}(specs.NumMipLevels));
				hash = Nano::Hash::Combine(hash, std::hash<decltype(specs.BaseArraySlice)>{}(specs.BaseArraySlice));
				hash = Nano::Hash::Combine(hash, std::hash<decltype(specs.NumArraySlices)>{}(specs.NumArraySlices));
				hash = Nano::Hash::Combine(hash, std::hash<std::underlying_type_t<decltype(viewType)>>{}(std::to_underlying(viewType)));
				hash = Nano::Hash::Combine(hash, std::hash<std::underlying_type_t<decltype(dimension)>>{}(std::to_underlying(dimension)));
				hash = Nano::Hash::Combine(hash, std::hash<std::underlying_type_t<decltype(format)>>{}(std::to_underlying(format)));
				hash = Nano::Hash::Combine(hash, std::hash<uint32_t>{}(usage));

				return hash;
			}
		};
	public:
		// Constructor & Destructor
		VulkanImageSubresourceView(const Image& image, const ImageSubresourceSpecification& specs);
		~VulkanImageSubresourceView();

		// Getters
		inline const ImageSubresourceSpecification& GetSpecification() const { return m_Specification; }

		// Internal getters
		inline VkImageView GetVkImageView() const { return m_ImageView; }

	private:
		const VulkanImage& m_Image;
		ImageSubresourceSpecification m_Specification;

		VkImageView m_ImageView = VK_NULL_HANDLE;

		friend class VulkanImage;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanImage
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanImage
	{
	public:
		// Constructors & Destructor
		VulkanImage(const Device& device, const ImageSpecification& specs);
		VulkanImage(const Device& device, const ImageSpecification& specs, VkImage image);
		~VulkanImage();

		// Methods
		const VulkanImageSubresourceView& GetSubresourceView(const ImageSubresourceSpecification& specs, ImageDimension dimension = ImageDimension::Unknown, Format format = Format::Unknown, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT, ImageSubresourceViewType viewType = ImageSubresourceViewType::AllAspects);

		// Getters
		inline const ImageSpecification& GetSpecification() const { return m_Specification; }

		// Internal getters
		inline VkImage GetVkImage() const { return m_Image; }
		inline VmaAllocation GetVmaAllocation() const { return m_Allocation; }

		inline std::unordered_map<VulkanImageSubresourceView::Key, VulkanImageSubresourceView, VulkanImageSubresourceView::Hash>& GetImageViews() { return m_ImageViews; }

	private:
		const VulkanDevice& m_Device;
		ImageSpecification m_Specification;

		ResourceState m_CurrentState = ResourceState::Unknown;

		VkImage m_Image = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;

		std::unordered_map<VulkanImageSubresourceView::Key, VulkanImageSubresourceView, VulkanImageSubresourceView::Hash> m_ImageViews = {};
	};

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanSampler
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanSampler
	{
	public:
		// Constructor & Destructor
		VulkanSampler(const Device& device, const SamplerSpecification& specs);
		~VulkanSampler();

		// Getters
		inline const SamplerSpecification& GetSpecification() const { return m_Specification; }

		// Internal getters
		inline VkSampler GetVkSampler() const { return m_Sampler; }

	private:
		const VulkanDevice& m_Device;
		SamplerSpecification m_Specification;

		VkSampler m_Sampler = VK_NULL_HANDLE;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// Conversion functions
	////////////////////////////////////////////////////////////////////////////////////
	inline constexpr VkImageType ImageDimensionToVkImageType(ImageDimension dimension)
	{
		switch (dimension)
		{
		case ImageDimension::Image1D:
		case ImageDimension::Image1DArray:
			return VK_IMAGE_TYPE_1D;

		case ImageDimension::Image2D:
		case ImageDimension::Image2DArray:
		case ImageDimension::ImageCube:
		case ImageDimension::ImageCubeArray:
		case ImageDimension::Image2DMS:
		case ImageDimension::Image2DMSArray:
			return VK_IMAGE_TYPE_2D;

		case ImageDimension::Image3D:
			return VK_IMAGE_TYPE_3D;

		default:
			break;
		}

		NG_UNREACHABLE();
		return VK_IMAGE_TYPE_2D;
	}

	inline constexpr VkImageViewType ImageDimensionToVkImageViewType(ImageDimension dimension)
	{
		switch (dimension)
		{
		case ImageDimension::Image1D:
			VK_IMAGE_VIEW_TYPE_1D;

		case ImageDimension::Image1DArray:
			VK_IMAGE_VIEW_TYPE_1D_ARRAY;

		case ImageDimension::Image2D:
		case ImageDimension::Image2DMS:
			return VK_IMAGE_VIEW_TYPE_2D;

		case ImageDimension::Image2DArray:
		case ImageDimension::Image2DMSArray:
			return VK_IMAGE_VIEW_TYPE_2D_ARRAY;

		case ImageDimension::ImageCube:
			return VK_IMAGE_VIEW_TYPE_CUBE;

		case ImageDimension::ImageCubeArray:
			return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;

		case ImageDimension::Image3D:
			return VK_IMAGE_VIEW_TYPE_3D;

		default:
			break;
		}

		NG_UNREACHABLE();
		return VK_IMAGE_VIEW_TYPE_2D;
	}

	inline constexpr VkFormat FormatToVkFormat(Format format)
	{
		switch (format)
		{
		case Format::Unknown:            return VK_FORMAT_UNDEFINED;
		case Format::R8UInt:             return VK_FORMAT_R8_UINT;
		case Format::R8SInt:             return VK_FORMAT_R8_SINT;
		case Format::R8Unorm:            return VK_FORMAT_R8_UNORM;
		case Format::R8Snorm:            return VK_FORMAT_R8_SNORM;
		case Format::RG8UInt:            return VK_FORMAT_R8G8_UINT;
		case Format::RG8SInt:            return VK_FORMAT_R8G8_SINT;
		case Format::RG8Unorm:           return VK_FORMAT_R8G8_UNORM;
		case Format::RG8Snorm:           return VK_FORMAT_R8G8_SNORM;
		case Format::R16UInt:            return VK_FORMAT_R16_UINT;
		case Format::R16SInt:            return VK_FORMAT_R16_SINT;
		case Format::R16Unorm:           return VK_FORMAT_R16_UNORM;
		case Format::R16Snorm:           return VK_FORMAT_R16_SNORM;
		case Format::R16Float:           return VK_FORMAT_R16_SFLOAT;
		case Format::BGRA4Unorm:         return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
		case Format::B5G6R5Unorm:        return VK_FORMAT_B5G6R5_UNORM_PACK16;
		case Format::B5G5R5A1Unorm:      return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
		case Format::RGBA8UInt:          return VK_FORMAT_R8G8B8A8_UINT;
		case Format::RGBA8SInt:          return VK_FORMAT_R8G8B8A8_SINT;
		case Format::RGBA8Unorm:         return VK_FORMAT_R8G8B8A8_UNORM;
		case Format::RGBA8Snorm:         return VK_FORMAT_R8G8B8A8_SNORM;
		case Format::BGRA8Unorm:         return VK_FORMAT_B8G8R8A8_UNORM;
		case Format::SRGBA8Unorm:        return VK_FORMAT_R8G8B8A8_SRGB;
		case Format::SBGRA8Unorm:        return VK_FORMAT_B8G8R8A8_SRGB;
		case Format::R10G10B10A2Unorm:   return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		case Format::R11G11B10Float:     return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		case Format::RG16UInt:           return VK_FORMAT_R16G16_UINT;
		case Format::RG16SInt:           return VK_FORMAT_R16G16_SINT;
		case Format::RG16Unorm:          return VK_FORMAT_R16G16_UNORM;
		case Format::RG16Snorm:          return VK_FORMAT_R16G16_SNORM;
		case Format::RG16Float:          return VK_FORMAT_R16G16_SFLOAT;
		case Format::R32UInt:            return VK_FORMAT_R32_UINT;
		case Format::R32SInt:            return VK_FORMAT_R32_SINT;
		case Format::R32Float:           return VK_FORMAT_R32_SFLOAT;
		case Format::RGBA16UInt:         return VK_FORMAT_R16G16B16A16_UINT;
		case Format::RGBA16SInt:         return VK_FORMAT_R16G16B16A16_SINT;
		case Format::RGBA16Float:        return VK_FORMAT_R16G16B16A16_SFLOAT;
		case Format::RGBA16Unorm:        return VK_FORMAT_R16G16B16A16_UNORM;
		case Format::RGBA16Snorm:        return VK_FORMAT_R16G16B16A16_SNORM;
		case Format::RG32UInt:           return VK_FORMAT_R32G32_UINT;
		case Format::RG32SInt:           return VK_FORMAT_R32G32_SINT;
		case Format::RG32Float:          return VK_FORMAT_R32G32_SFLOAT;
		case Format::RGB32UInt:          return VK_FORMAT_R32G32B32_UINT;
		case Format::RGB32SInt:          return VK_FORMAT_R32G32B32_SINT;
		case Format::RGB32Float:         return VK_FORMAT_R32G32B32_SFLOAT;
		case Format::RGBA32UInt:         return VK_FORMAT_R32G32B32A32_UINT;
		case Format::RGBA32SInt:         return VK_FORMAT_R32G32B32A32_SINT;
		case Format::RGBA32Float:        return VK_FORMAT_R32G32B32A32_SFLOAT;

		case Format::D16:                return VK_FORMAT_D16_UNORM;
		case Format::D24S8:              return VK_FORMAT_D24_UNORM_S8_UINT;
		case Format::X24G8UInt:          return VK_FORMAT_D24_UNORM_S8_UINT;
		case Format::D32:                return VK_FORMAT_D32_SFLOAT;
		case Format::D32S8:              return VK_FORMAT_D32_SFLOAT_S8_UINT;
		case Format::X32G8UInt:          return VK_FORMAT_D32_SFLOAT_S8_UINT;

		case Format::BC1Unorm:           return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
		case Format::BC1UnormSRGB:       return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
		case Format::BC2Unorm:           return VK_FORMAT_BC2_UNORM_BLOCK;
		case Format::BC2UnormSRGB:       return VK_FORMAT_BC2_SRGB_BLOCK;
		case Format::BC3Unorm:           return VK_FORMAT_BC3_UNORM_BLOCK;
		case Format::BC3UnormSRGB:       return VK_FORMAT_BC3_SRGB_BLOCK;
		case Format::BC4Unorm:           return VK_FORMAT_BC4_UNORM_BLOCK;
		case Format::BC4Snorm:           return VK_FORMAT_BC4_SNORM_BLOCK;
		case Format::BC5Unorm:           return VK_FORMAT_BC5_UNORM_BLOCK;
		case Format::BC5Snorm:           return VK_FORMAT_BC5_SNORM_BLOCK;
		case Format::BC6HUFloat:         return VK_FORMAT_BC6H_UFLOAT_BLOCK;
		case Format::BC6HSFloat:         return VK_FORMAT_BC6H_SFLOAT_BLOCK;
		case Format::BC7Unorm:           return VK_FORMAT_BC7_UNORM_BLOCK;
		case Format::BC7UnormSRGB:       return VK_FORMAT_BC7_SRGB_BLOCK;

		default:
			break;
		}

		NG_UNREACHABLE();
		return VK_FORMAT_UNDEFINED;
	}

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

	inline constexpr VkFilter FilterModeToVkFilter(FilterMode filter)
	{
		switch (filter)
		{
		case FilterMode::Nearest:		return VK_FILTER_NEAREST;
		case FilterMode::Linear:		return VK_FILTER_LINEAR;

		default:
			break;
		}

		NG_UNREACHABLE();
		return VK_FILTER_NEAREST;
	}

	inline constexpr VkSamplerAddressMode SamplerAddressModeToVkSamplerAddressMode(SamplerAddressMode mode)
	{
		switch (mode)
		{
		case SamplerAddressMode::ClampToEdge:			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case SamplerAddressMode::Repeat:				return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case SamplerAddressMode::ClampToBorder:			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case SamplerAddressMode::MirroredRepeat:		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case SamplerAddressMode::MirrorClampToEdge:		return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;

		default:
			break;
		}

		NG_UNREACHABLE();
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
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

}