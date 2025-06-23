#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Resources.hpp"

#include <utility>

namespace Nano::Graphics
{
	class Device;
}

namespace Nano::Graphics::Internal
{

	class Dx12Device;
	class Dx12Image;
	class Dx12StagingImage;
	class Dx12Sampler;

#if defined(NG_API_DX12)
	////////////////////////////////////////////////////////////////////////////////////
	// Dx12ImageSubresourceView
	////////////////////////////////////////////////////////////////////////////////////
	class Dx12ImageSubresourceView
	{
	public:
		using Key = std::tuple<ImageSubresourceSpecification, ImageSubresourceViewUsage, ImageDimension, Format>;
		struct Hash
		{
		public:
			uint64_t operator()(const Key& key) const noexcept(true)
			{
				const auto& [specs, dimension, format, viewUsage] = key;
	
				uint64_t hash = 0;
	
				hash = Nano::Hash::Combine(hash, std::hash<decltype(specs.BaseMipLevel)>{}(specs.BaseMipLevel));
				hash = Nano::Hash::Combine(hash, std::hash<decltype(specs.NumMipLevels)>{}(specs.NumMipLevels));
				hash = Nano::Hash::Combine(hash, std::hash<decltype(specs.BaseArraySlice)>{}(specs.BaseArraySlice));
				hash = Nano::Hash::Combine(hash, std::hash<decltype(specs.NumArraySlices)>{}(specs.NumArraySlices));
				hash = Nano::Hash::Combine(hash, std::hash<std::underlying_type_t<decltype(dimension)>>{}(std::to_underlying(dimension)));
				hash = Nano::Hash::Combine(hash, std::hash<std::underlying_type_t<decltype(format)>>{}(std::to_underlying(format)));
				hash = Nano::Hash::Combine(hash, std::hash<std::underlying_type_t<decltype(viewUsage)>>{}(std::to_underlying(viewUsage)));
	
				return hash;
			}
		};
	public:
		// Constructor & Destructor
		Dx12ImageSubresourceView(const Image& image, const ImageSubresourceSpecification& specs);
		~Dx12ImageSubresourceView();
	
		// Getters
		inline const ImageSubresourceSpecification& GetSpecification() const { return m_Specification; }
	
		// Internal getters
		inline CD3DX12_CPU_DESCRIPTOR_HANDLE GetHandle() const { return m_Handle; }
	
	private:
		const Dx12Image& m_Image;
		ImageSubresourceSpecification m_Specification;
	
		CD3DX12_CPU_DESCRIPTOR_HANDLE m_Handle = {};
	
		friend class Dx12Image;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// Dx12Image
	////////////////////////////////////////////////////////////////////////////////////
	class Dx12Image
	{
	public:
		// Constructors & Destructor
		Dx12Image(const Device& device);
		Dx12Image(const Device& device, const ImageSpecification& specs);
		~Dx12Image();

		// Getters
		inline const ImageSpecification& GetSpecification() const { return m_Specification; }

		// Internal methods
		void SetInternalData(const ImageSpecification& specs, DxPtr<ID3D12Resource> image);

		// Internal getters
		inline DxPtr<ID3D12Resource> GetD3D12Resource() const { return m_Resource; }

		const Dx12ImageSubresourceView& GetSubresourceView(const ImageSubresourceSpecification& specs, ImageSubresourceViewUsage usage, ImageDimension dimension = ImageDimension::Unknown, Format format = Format::Unknown);
		inline std::unordered_map<Dx12ImageSubresourceView::Key, Dx12ImageSubresourceView, Dx12ImageSubresourceView::Hash>& GetImageViews() { return m_ImageViews; }

	private:
		const Dx12Device& m_Device;
		ImageSpecification m_Specification;

		DxPtr<ID3D12Resource> m_Resource = nullptr;

		std::unordered_map<Dx12ImageSubresourceView::Key, Dx12ImageSubresourceView, Dx12ImageSubresourceView::Hash> m_ImageViews = {};

		friend class Dx12Device;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// Dx12StagingImage
	////////////////////////////////////////////////////////////////////////////////////
	//class Dx12StagingImage
	//{
	//public:
	//	struct Region
	//	{
	//	public:
	//		size_t Offset;
	//		size_t Size;
	//	};
	//public:
	//	// Constructor & Destructor
	//	VulkanStagingImage(const Device& device, const ImageSpecification& specs, CpuAccessMode cpuAccessMode);
	//	~VulkanStagingImage();
	//
	//	// Getters
	//	inline const ImageSpecification& GetSpecification() const { return m_Specification; }
	//
	//	// Internal getters
	//	inline VulkanBuffer& GetVulkanBuffer() { return m_Buffer; }
	//	inline const VulkanBuffer& GetVulkanBuffer() const { return m_Buffer; }
	//
	//	VulkanStagingImage::Region GetSliceRegion(MipLevel mipLevel, ArraySlice arraySlice, uint32_t z);
	//
	//private:
	//	// Private methods
	//	std::vector<Region> GetSliceRegions() const;
	//	size_t ComputeSliceSize(uint32_t mipLevel) const;
	//
	//	size_t GetBufferSize() const;
	//
	//private:
	//	const VulkanDevice& m_Device;
	//	ImageSpecification m_Specification;
	//
	//	std::vector<Region> m_SliceRegions = {};
	//
	//	VulkanBuffer m_Buffer;
	//};

	////////////////////////////////////////////////////////////////////////////////////
	// Dx12Sampler
	////////////////////////////////////////////////////////////////////////////////////
	//class Dx12Sampler
	//{
	//public:
	//	// Constructor & Destructor
	//	Dx12Sampler(const Device& device, const SamplerSpecification& specs);
	//	~Dx12Sampler();
	//
	//	// Getters
	//	inline const SamplerSpecification& GetSpecification() const { return m_Specification; }
	//
	//	// Internal getters
	//	inline VkSampler GetVkSampler() const { return m_Sampler; }
	//
	//private:
	//	const VulkanDevice& m_Device;
	//	SamplerSpecification m_Specification;
	//
	//	VkSampler m_Sampler = VK_NULL_HANDLE;
	//};
#endif

}