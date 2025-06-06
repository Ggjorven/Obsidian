#pragma once

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

	private:
		const VulkanImage& m_Image;
		ImageSubresourceSpecification m_Specification;

		VkImageView m_ImageView = VK_NULL_HANDLE;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanImage
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanImage
	{
	public:
		// Constructor & Destructor
		VulkanImage(const Device& device, const ImageSpecification& specs);
		~VulkanImage();

		// Getters
		inline const ImageSpecification& GetSpecification() const { return m_Specification; }

	private:

	private:
		const VulkanDevice& m_Device;
		ImageSpecification m_Specification;

		ResourceState m_CurrentState = ResourceState::Unknown;

		VkImage m_Image = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = VK_NULL_HANDLE;

		std::unordered_map<VulkanImageSubresourceView::Key, VulkanImageSubresourceView, VulkanImageSubresourceView::Hash> m_ImageViews = {};
	};

}