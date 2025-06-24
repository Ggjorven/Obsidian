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
	CD3DX12_CPU_DESCRIPTOR_HANDLE Dx12ImageSubresourceView::GetHandle() const
	{
		switch (m_Usage)
		{
		case ImageSubresourceViewUsage::SRV:
		case ImageSubresourceViewUsage::UAV:
			return m_Image.GetDx12Device().GetResources().GetSRVAndUAVHeap().GetHandleForIndex(m_Index);
		
		case ImageSubresourceViewUsage::RTV:
			return m_Image.GetDx12Device().GetResources().GetRTVHeap().GetHandleForIndex(m_Index);

		case ImageSubresourceViewUsage::DSV:
			return m_Image.GetDx12Device().GetResources().GetDSVHeap().GetHandleForIndex(m_Index);

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
		m_Allocation = m_Device.GetAllocator().CreateImage(m_Resource, ResourceStateToD3D12ResourceStates(m_Specification.PermanentState), ImageSpecificationToD3D12ResourceDesc(m_Specification), D3D12_HEAP_TYPE_DEFAULT);
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
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Internal getters
	////////////////////////////////////////////////////////////////////////////////////
	const Dx12ImageSubresourceView& Dx12Image::GetSubresourceView(const ImageSubresourceSpecification& specs, ImageSubresourceViewUsage usage, ImageDimension dimension, Format format)
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
			imageView.m_Index = m_Device.GetResources().GetSRVAndUAVHeap().CreateSRV(format, dimension, specs, m_Specification, m_Resource.Get());
			break;
		case ImageSubresourceViewUsage::UAV:
			imageView.m_Index = m_Device.GetResources().GetSRVAndUAVHeap().CreateUAV(format, dimension, specs, m_Specification, m_Resource.Get());
			break;
		case ImageSubresourceViewUsage::RTV:
			imageView.m_Index = m_Device.GetResources().GetRTVHeap().CreateRTV(format, specs, m_Specification, m_Resource.Get());
			break;
		case ImageSubresourceViewUsage::DSV:
			imageView.m_Index = m_Device.GetResources().GetDSVHeap().CreateDSV(specs, m_Specification, m_Resource.Get());
			break;

		default:
			break;
		}

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

		return { };
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
		D3D12_SAMPLER_DESC samplerDesc = {};

		UINT reductionType = static_cast<UINT>(SamplerReductionTypeToD3D12FilterReductionType(m_Specification.ReductionType));
		if (m_Specification.MaxAnisotropy > 1.0f)
			samplerDesc.Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(reductionType);
		else
			samplerDesc.Filter = D3D12_ENCODE_BASIC_FILTER(
				FilterModeToD3D12FilterType(m_Specification.MinFilter),
				FilterModeToD3D12FilterType(m_Specification.MagFilter),
				FilterModeToD3D12FilterType(m_Specification.MipFilter),
				reductionType
			);

		samplerDesc.AddressU = SamplerAddresModeToD3D12TextureAddressMode(m_Specification.AddressU);
		samplerDesc.AddressV = SamplerAddresModeToD3D12TextureAddressMode(m_Specification.AddressV);
		samplerDesc.AddressW = SamplerAddresModeToD3D12TextureAddressMode(m_Specification.AddressW);
		samplerDesc.MipLODBias = m_Specification.MipBias;
		samplerDesc.MaxAnisotropy = std::max(static_cast<UINT>(m_Specification.MaxAnisotropy), 1u);
		samplerDesc.ComparisonFunc = ((m_Specification.ReductionType == SamplerReductionType::Comparison) ? D3D12_COMPARISON_FUNC_LESS : D3D12_COMPARISON_FUNC_NEVER);
		samplerDesc.BorderColor[0] = m_Specification.BorderColour.r;
		samplerDesc.BorderColor[1] = m_Specification.BorderColour.g;
		samplerDesc.BorderColor[2] = m_Specification.BorderColour.b;
		samplerDesc.BorderColor[3] = m_Specification.BorderColour.a;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		
		m_SamplerIndex = m_Device.GetResources().GetSamplerHeap().CreateSampler(samplerDesc);
	}

	Dx12Sampler::~Dx12Sampler()
	{
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Internal getters
	////////////////////////////////////////////////////////////////////////////////////
	CD3DX12_CPU_DESCRIPTOR_HANDLE Dx12Sampler::GetHandle() const
	{
		return m_Device.GetResources().GetSamplerHeap().GetHandleForIndex(m_SamplerIndex);
	}

}