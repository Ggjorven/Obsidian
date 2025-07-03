#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Buffer.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Resources.hpp"

#include <utility>

namespace Nano::Graphics
{
	class Device;
	class Image;
}

namespace Nano::Graphics::Internal
{

	class Dx12Device;
	class Dx12Image;
	class Dx12StagingImage;
	class Dx12Sampler;
	class Dx12Swapchain;

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
		Dx12ImageSubresourceView(const Image& image, const ImageSubresourceSpecification& specs, ImageSubresourceViewUsage usage);
		~Dx12ImageSubresourceView();
	
		// Getters
		inline const ImageSubresourceSpecification& GetSpecification() const { return m_Specification; }
		inline ImageSubresourceViewUsage GetUsage() const { return m_Usage; }
	
		// Internal getters
		inline DescriptorHeapIndex GetIndex() const { return m_Index; }
		CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const;
	
	private:
		const Dx12Image& m_Image;
		ImageSubresourceSpecification m_Specification;
		ImageSubresourceViewUsage m_Usage;

		DescriptorHeapIndex m_Index = {};
	
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
		inline const Dx12Device& GetDx12Device() const { return m_Device; }

		inline DxPtr<ID3D12Resource> GetD3D12Resource() const { return m_Resource; }
		inline DxPtr<D3D12MA::Allocation> GetD3D12MAAllocation() const { return m_Allocation; }

		const Dx12ImageSubresourceView& GetSubresourceView(const ImageSubresourceSpecification& specs, ImageSubresourceViewUsage usage, ImageDimension dimension = ImageDimension::Unknown, Format format = Format::Unknown, bool isReadOnly = false); // Note: For RTV & DSV
		const Dx12ImageSubresourceView& GetSubresourceView(DescriptorHeapIndex index, const ImageSubresourceSpecification& specs, ImageSubresourceViewUsage usage, ImageDimension dimension = ImageDimension::Unknown, Format format = Format::Unknown);
		inline std::unordered_map<Dx12ImageSubresourceView::Key, Dx12ImageSubresourceView, Dx12ImageSubresourceView::Hash>& GetImageViews() { return m_ImageViews; }

		inline uint8_t GetPlaneCount() const { return m_PlaneCount; }

	private:
		const Dx12Device& m_Device;
		ImageSpecification m_Specification;

		DxPtr<ID3D12Resource> m_Resource = nullptr;
		DxPtr<D3D12MA::Allocation> m_Allocation = nullptr;

		std::unordered_map<Dx12ImageSubresourceView::Key, Dx12ImageSubresourceView, Dx12ImageSubresourceView::Hash> m_ImageViews = {};

		// Dx12 needed info
		uint8_t m_PlaneCount = 0;

		friend class Dx12Device;
		friend class Dx12Swapchain;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// Dx12StagingImage
	////////////////////////////////////////////////////////////////////////////////////
	class Dx12StagingImage
	{
	public:
		struct SliceRegion
		{
		public:
			size_t Offset = 0;
			size_t Size = 0;

			D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint = {};
		};
	public:
		// Constructor & Destructor
		Dx12StagingImage(const Device& device, const ImageSpecification& specs, CpuAccessMode cpuAccessMode);
		~Dx12StagingImage();
	
		// Getters
		inline const ImageSpecification& GetSpecification() const { return m_Specification; }
	
		// Internal getters
		inline Dx12Buffer& GetDx12Buffer() { return m_Buffer; }
		inline const Dx12Buffer& GetDx12Buffer() const { return m_Buffer; }

		SliceRegion GetSliceRegion(const ImageSliceSpecification& slice) const;
	
	private:
		// Private methods
		std::vector<UINT64> GetSubresourceOffsets() const;
		size_t GetBufferSize() const;
	
	private:
		const Dx12Device& m_Device;
		ImageSpecification m_Specification;
	
		std::vector<UINT64> m_SubresourceOffsets = {};
	
		Dx12Buffer m_Buffer;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// Dx12Sampler
	////////////////////////////////////////////////////////////////////////////////////
	class Dx12Sampler
	{
	public:
		// Constructor & Destructor
		Dx12Sampler(const Device& device, const SamplerSpecification& specs);
		~Dx12Sampler();
	
		// Getters
		inline const SamplerSpecification& GetSpecification() const { return m_Specification; }
	
	private:
		const Dx12Device& m_Device;
		SamplerSpecification m_Specification;

		friend class Dx12Device;
	};
#endif

}