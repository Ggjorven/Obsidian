#pragma once

#include "NanoGraphics/Core/Core.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanResources.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanBuffer.hpp"

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
	class VulkanImageSubresourceView;
	class VulkanImage;
	class VulkanStagingImage;
	class VulkanSampler;

#if defined(NG_API_VULKAN)
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
	class VulkanImage : public Traits::NoCopy
	{
	public:
		// Constructors & Destructor
		VulkanImage(const Device& device, const ImageSpecification& specs);
		VulkanImage(const VulkanDevice& device, const ImageSpecification& specs, VkImage image);
		~VulkanImage();

		// Getters
		inline const ImageSpecification& GetSpecification() const { return m_Specification; }

		// Internal methods
		void SetInternalData(const ImageSpecification& specs, VkImage image);

		// Internal getters
		inline VkImage GetVkImage() const { return m_Image; }
		inline VmaAllocation GetVmaAllocation() const { return m_Allocation; }

		const VulkanImageSubresourceView& GetSubresourceView(const ImageSubresourceSpecification& specs, ImageDimension dimension = ImageDimension::Unknown, Format format = Format::Unknown, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT, ImageSubresourceViewType viewType = ImageSubresourceViewType::AllAspects);
		inline std::unordered_map<VulkanImageSubresourceView::Key, VulkanImageSubresourceView, VulkanImageSubresourceView::Hash>& GetImageViews() { return m_ImageViews; }

	private:
		const VulkanDevice& m_Device;
		ImageSpecification m_Specification;

		VkImage m_Image = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;

		std::unordered_map<VulkanImageSubresourceView::Key, VulkanImageSubresourceView, VulkanImageSubresourceView::Hash> m_ImageViews = {};
	};

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanStagingImage
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanStagingImage : public Traits::NoCopy
	{
	public:
		struct Region
		{
		public:
			size_t Offset;
			size_t Size;
		};
	public:
		// Constructor & Destructor
		VulkanStagingImage(const Device& device, const ImageSpecification& specs, CpuAccessMode cpuAccessMode);
		~VulkanStagingImage();

		// Getters
		inline const ImageSpecification& GetSpecification() const { return m_Specification; }

		// Internal getters
		inline VulkanBuffer& GetVulkanBuffer() { return m_Buffer; }
		inline const VulkanBuffer& GetVulkanBuffer() const { return m_Buffer; }

		VulkanStagingImage::Region GetSliceRegion(MipLevel mipLevel, ArraySlice arraySlice, uint32_t z);

	private:
		// Private methods
		std::vector<Region> GetSliceRegions() const;
		size_t ComputeSliceSize(uint32_t mipLevel) const;

		size_t GetBufferSize() const;

	private:
		const VulkanDevice& m_Device;
		ImageSpecification m_Specification;

		std::vector<Region> m_SliceRegions = {};

		VulkanBuffer m_Buffer;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanSampler
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanSampler : public Traits::NoCopy
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
#endif

}