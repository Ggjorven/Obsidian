#include "ngpch.h"
#include "Dx12Image.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12Device.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Resources.hpp"

namespace Nano::Graphics::Internal
{

	namespace
	{

		////////////////////////////////////////////////////////////////////////////////////
		// Helper method 
		////////////////////////////////////////////////////////////////////////////////////
		D3D12_RESOURCE_DESC ImageSpecificationToD3D12ResourceDesc(const ImageSpecification& specs)
		{
			// Note: Copied from Dx12.hpp:CreateImage
			D3D12_RESOURCE_DESC resourceDesc = {};
			resourceDesc.Dimension = ImageDimensionToD3D12ResourceDimension(specs.Dimension);
			resourceDesc.Alignment = 0;
			resourceDesc.Width = specs.Width;
			resourceDesc.Height = specs.Height;
			resourceDesc.DepthOrArraySize = static_cast<UINT16>(((specs.Dimension == ImageDimension::Image3D) ? specs.Depth : specs.ArraySize));
			resourceDesc.MipLevels = static_cast<UINT16>(specs.MipLevels);
			resourceDesc.Format = (specs.IsTypeless ? FormatToFormatMapping(specs.ImageFormat).ResourceFormat : FormatToFormatMapping(specs.ImageFormat).RTVFormat);
			resourceDesc.SampleDesc.Count = specs.SampleCount;
			resourceDesc.SampleDesc.Quality = specs.SampleQuality;
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			
			FormatInfo formatInfo = FormatToFormatInfo(specs.ImageFormat);

			if (!specs.IsShaderResource)
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

			if (specs.IsRenderTarget)
			{
				if (formatInfo.HasDepth || formatInfo.HasStencil)
					resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
				else
					resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
			}

			if (specs.IsUnorderedAccessed)
				resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

			return resourceDesc;
		}

	}

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
	Dx12ImageSubresourceView::Dx12ImageSubresourceView(const Image& image, const ImageSubresourceSpecification& specs, ImageSubresourceViewUsage usage)
		: m_Image(*api_cast<const Dx12Image*>(&image)), m_Specification(specs), m_Usage(usage)
	{
	}

	Dx12ImageSubresourceView::~Dx12ImageSubresourceView()
	{
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Internal getters
	////////////////////////////////////////////////////////////////////////////////////
	CD3DX12_CPU_DESCRIPTOR_HANDLE Dx12ImageSubresourceView::GetCPUHandle() const
	{
		switch (m_Usage)
		{
		case ImageSubresourceViewUsage::SRV:
		case ImageSubresourceViewUsage::UAV:
			return m_Image.GetDx12Device().GetResources().GetSRVAndUAVHeap().GetCPUHandleForIndex(m_Index);
		
		case ImageSubresourceViewUsage::RTV:
			return m_Image.GetDx12Device().GetResources().GetRTVHeap().GetCPUHandleForIndex(m_Index);

		case ImageSubresourceViewUsage::DSV:
			return m_Image.GetDx12Device().GetResources().GetDSVHeap().GetCPUHandleForIndex(m_Index);

		default:
			NG_UNREACHABLE();
			break;
		}

		return {};
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
	Dx12Image::Dx12Image(const Device& device)
		: m_Device(*api_cast<const Dx12Device*>(&device)), m_Specification({})
	{
	}

	Dx12Image::Dx12Image(const Device& device, const ImageSpecification& specs)
		: m_Device(*api_cast<const Dx12Device*>(&device)), m_Specification(specs)
	{
		NG_ASSERT(((m_Specification.Width != 0) && (m_Specification.Height != 0)), "[Dx12Image] Invalid width/height passed in.");
		NG_ASSERT((m_Specification.ImageFormat != Format::Unknown), "[Dx12Image] Invalid format passed in.");

		m_Allocation = m_Device.GetAllocator().CreateImage(m_Resource, ResourceStateToD3D12ResourceStates(m_Specification.PermanentState), ImageSpecificationToD3D12ResourceDesc(m_Specification), D3D12_HEAP_TYPE_DEFAULT);
	
		m_PlaneCount = Dx12FormatToPlaneCount(device, (m_Specification.IsTypeless ? FormatToFormatMapping(m_Specification.ImageFormat).ResourceFormat : FormatToFormatMapping(m_Specification.ImageFormat).RTVFormat));

		if constexpr (Information::Validation)
		{
			if (!m_Specification.DebugName.empty())
				m_Device.GetContext().SetDebugName(m_Resource.Get(), m_Specification.DebugName);
		}
	}

	Dx12Image::~Dx12Image()
	{
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Internal methods
	////////////////////////////////////////////////////////////////////////////////////
	void Dx12Image::SetInternalData(const ImageSpecification& specs, DxPtr<ID3D12Resource> image)
	{
		m_Specification = specs;
		m_Resource = image;

		m_PlaneCount = Dx12FormatToPlaneCount(*api_cast<const Device*>(&m_Device), (m_Specification.IsTypeless ? FormatToFormatMapping(m_Specification.ImageFormat).ResourceFormat : FormatToFormatMapping(m_Specification.ImageFormat).RTVFormat));
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Internal getters
	////////////////////////////////////////////////////////////////////////////////////
	const Dx12ImageSubresourceView& Dx12Image::GetSubresourceView(const ImageSubresourceSpecification& specs, ImageSubresourceViewUsage usage, ImageDimension dimension, Format format, bool isReadOnly)
	{
		// Automatically set the dimension and format if not specified
		if (dimension == ImageDimension::Unknown)
			dimension = m_Specification.Dimension;
		if (format == Format::Unknown)
			format = m_Specification.ImageFormat;

		// Find the view in map
		auto cachekey = std::make_tuple(specs, usage, dimension, format);
		auto it = m_ImageViews.find(cachekey);
		if (it != m_ImageViews.end())
			return it->second;

		// Create new
		auto viewPair = m_ImageViews.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(cachekey),
			std::forward_as_tuple(*api_cast<Image*>(this), specs, usage)
		);
		auto& imageView = std::get<0>(viewPair)->second;

		switch (usage)
		{
		case ImageSubresourceViewUsage::RTV:
			imageView.m_Index = m_Device.GetResources().GetRTVHeap().CreateRTV(m_Specification, specs, m_Resource.Get(), format);
			break;
		case ImageSubresourceViewUsage::DSV:
			imageView.m_Index = m_Device.GetResources().GetDSVHeap().CreateDSV(m_Specification, specs, m_Resource.Get(), isReadOnly);
			break;

		default:
			NG_ASSERT(false, "[VkImage] Trying to get imageview without index for SRV or UAV, these are regular DescriptorHeaps so dynamic indexing is not implemented.");
			break;
		}

		return imageView;
	}

	const Dx12ImageSubresourceView& Dx12Image::GetSubresourceView(DescriptorHeapIndex index, const ImageSubresourceSpecification& specs, ImageSubresourceViewUsage usage, ImageDimension dimension, Format format)
	{
		// Automatically set the dimension and format if not specified
		if (dimension == ImageDimension::Unknown)
			dimension = m_Specification.Dimension;
		if (format == Format::Unknown)
			format = m_Specification.ImageFormat;

		// Find the view in map
		auto cachekey = std::make_tuple(specs, usage, dimension, format);
		auto it = m_ImageViews.find(cachekey);
		if (it != m_ImageViews.end())
			return it->second;

		// Create new
		auto viewPair = m_ImageViews.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(cachekey),
			std::forward_as_tuple(*api_cast<Image*>(this), specs, usage)
		);
		auto& imageView = std::get<0>(viewPair)->second;

		switch (usage)
		{
		case ImageSubresourceViewUsage::SRV:
			m_Device.GetResources().GetSRVAndUAVHeap().CreateSRV(index, m_Specification, specs, m_Resource.Get(), format, dimension);
			break;
		case ImageSubresourceViewUsage::UAV:
			m_Device.GetResources().GetSRVAndUAVHeap().CreateUAV(index, m_Specification, specs, m_Resource.Get(), format, dimension);
			break;

		default:
			NG_ASSERT(false, "[VkImage] Trying to get imageview with index for RTV or DSV, these are DynamicDescriptorHeaps so manual indexing is not permitted.");
			break;
		}

		imageView.m_Index = index;
		return imageView;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
	Dx12StagingImage::Dx12StagingImage(const Device& device, const ImageSpecification& specs, CpuAccessMode cpuAccessMode)
		: m_Device(*api_cast<const Dx12Device*>(&device)), m_Specification(specs), m_SubresourceOffsets(GetSubresourceOffsets()), m_Buffer(device, BufferSpecification()
			.SetSize(GetBufferSize())
			.SetCPUAccess(cpuAccessMode)
			.SetPermanentState(specs.PermanentState)
			.SetDebugName(specs.DebugName)
		)
	{
	}

	Dx12StagingImage::~Dx12StagingImage()
	{
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Private methods
	////////////////////////////////////////////////////////////////////////////////////
	std::vector<UINT64> Dx12StagingImage::GetSubresourceOffsets() const
	{
		UINT lastSubresource = CalculateSubresource(m_Specification.MipLevels - 1, m_Specification.ArraySize - 1, 0, m_Specification.MipLevels, m_Specification.ArraySize);
		UINT numSubresources = lastSubresource + 1;

		std::vector<UINT64> subresourceOffsets;
		subresourceOffsets.resize(numSubresources);

		D3D12_RESOURCE_DESC resourceDesc = ImageSpecificationToD3D12ResourceDesc(m_Specification);

		UINT64 baseOffset = 0;
		for (UINT i = 0; i < lastSubresource + 1; i++)
		{
			UINT64 subresourceSize;
			m_Device.GetContext().GetD3D12Device()->GetCopyableFootprints(&resourceDesc, i, 1, 0, nullptr, nullptr, nullptr, &subresourceSize);

			subresourceOffsets[i] = baseOffset;
			baseOffset += subresourceSize;
			baseOffset = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT * ((baseOffset + D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1) / D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
		}

		return subresourceOffsets;
	}

	size_t Dx12StagingImage::GetBufferSize() const
	{
		UINT lastSubresource = CalculateSubresource(m_Specification.MipLevels - 1, m_Specification.ArraySize - 1, 0, m_Specification.MipLevels, m_Specification.ArraySize);
		NG_ASSERT((lastSubresource < m_SubresourceOffsets.size()), "[Dx12StagingImage] Subresource count doesn't add up.");
		
		D3D12_RESOURCE_DESC resourceDesc = ImageSpecificationToD3D12ResourceDesc(m_Specification);

		// Compute size of last subresource
		UINT64 lastSubresourceSize;
		m_Device.GetContext().GetD3D12Device()->GetCopyableFootprints(&resourceDesc, lastSubresource, 1, 0, nullptr, nullptr, nullptr, &lastSubresourceSize);
		
		return m_SubresourceOffsets[lastSubresource] + lastSubresourceSize;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
	Dx12Sampler::Dx12Sampler(const Device& device, const SamplerSpecification& specs)
		: m_Device(*api_cast<const Dx12Device*>(&device)), m_Specification(specs)
	{
	}

	Dx12Sampler::~Dx12Sampler()
	{
	}

}