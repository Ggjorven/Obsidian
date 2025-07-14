#include "ngpch.h"
#include "Dx12Descriptors.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/ResourceSpec.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Device.hpp"

namespace Nano::Graphics::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
	Dx12DescriptorHeap::Dx12DescriptorHeap(const Device& device, uint32_t maxSize, D3D12_DESCRIPTOR_HEAP_TYPE type, bool isShaderVisible, const std::string& debugName)
		: m_Device(*api_cast<const Dx12Device*>(&device)), m_MaxSize(maxSize), m_Type(type), m_IsShaderVisible(isShaderVisible)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = maxSize;
		heapDesc.Type = m_Type;
		heapDesc.Flags = (m_IsShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		DX_VERIFY(m_Device.GetContext().GetD3D12Device()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DescriptorHeap)));

		// Caching data
		m_DescriptorSize = m_Device.GetContext().GetD3D12Device()->GetDescriptorHandleIncrementSize(type);
		m_CPUStart = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();

		if (m_IsShaderVisible) // GPU Descriptor are only avaible for shader visible heaps
			m_GPUStart = m_DescriptorHeap->GetGPUDescriptorHandleForHeapStart();

		if constexpr (Information::Validation)
		{
			if (!debugName.empty())
				m_Device.GetContext().SetDebugName(m_DescriptorHeap.Get(), debugName);
		}
	}

	Dx12DescriptorHeap::~Dx12DescriptorHeap()
	{
		m_Device.GetContext().Destroy([heap = m_DescriptorHeap]() {}); // Note: Holding a reference to the resource is enough to keep it alive (and destroy when the scope ends)
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Creation methods
	////////////////////////////////////////////////////////////////////////////////////
	void Dx12DescriptorHeap::CreateSRV(DescriptorHeapIndex index, const BufferSpecification& specs, const BufferRange& range, ResourceType type, ID3D12Resource* resource, Format format)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), "[Dx12DescriptorHeap] Cannot allocate an SRV from a non SRV heap.");
		NG_ASSERT(resource, "[Dx12DescriptorHeap] Resource must not be null.");

		BufferRange resRange = ResolveBufferRange(range, specs);

		if (format == Format::Unknown)
			format = specs.BufferFormat;

		FormatInfo info = FormatToFormatInfo(format);
		FormatMapping mapping = FormatToFormatMapping(format);

		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		switch (type)
		{
		case ResourceType::StructuredBufferSRV:
		{
			NG_ASSERT(specs.Size != 0, "[Dx12DescriptorHeap] Buffer size cannot be equal to 0.");
			NG_ASSERT(specs.Stride != 0, "[Dx12DescriptorHeap] Buffer stride cannot be equal to 0.");

			viewDesc.Format = DXGI_FORMAT_UNKNOWN;
			viewDesc.Buffer.FirstElement = range.Offset / specs.Stride;
			viewDesc.Buffer.NumElements = static_cast<UINT>(range.Size / specs.Stride);
			viewDesc.Buffer.StructureByteStride = static_cast<UINT>(specs.Stride);
			break;
		}

		//case ResourceType::TypedBufferSRV:
		//{
		//	NG_ASSERT(format != Format::Unknown, "[Dx12DescriptorHeap] ...");
		//
		//	viewDesc.Format = mapping.SRVFormat;
		//	viewDesc.Buffer.FirstElement = range.Offset / info.BytesPerBlock;
		//	viewDesc.Buffer.NumElements = static_cast<UINT>(range.Size / info.BytesPerBlock);
		//	break;
		//}

		default:
			NG_UNREACHABLE();
			return;
		}
		
		// Passthrough to other func
		CreateSRV(index, viewDesc, resource);
	}

	void Dx12DescriptorHeap::CreateSRV(DescriptorHeapIndex index, const ImageSpecification& specs, const ImageSubresourceSpecification& subresources, ID3D12Resource* resource, Format format, ImageDimension dimension)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), "[Dx12DescriptorHeap] Cannot allocate an SRV from a non SRV heap.");
		NG_ASSERT(resource, "[Dx12DescriptorHeap] Resource must not be null.");

		ImageSubresourceSpecification resSubresources = ResolveImageSubresource(subresources, specs, false);

		if (dimension == ImageDimension::Unknown)
			dimension = specs.Dimension;
		if (format == Format::Unknown)
			format = specs.ImageFormat;

		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.Format = FormatToFormatMapping(format).SRVFormat;
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		uint32_t planeSlice = (viewDesc.Format == DXGI_FORMAT_X24_TYPELESS_G8_UINT) ? 1 : 0;

		switch (dimension)
		{
		case ImageDimension::Image1D:
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			viewDesc.Texture1D.MostDetailedMip = resSubresources.BaseMipLevel;
			viewDesc.Texture1D.MipLevels = resSubresources.NumMipLevels;
			break;
		case ImageDimension::Image1DArray:
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
			viewDesc.Texture1DArray.FirstArraySlice = resSubresources.BaseArraySlice;
			viewDesc.Texture1DArray.ArraySize = resSubresources.NumArraySlices;
			viewDesc.Texture1DArray.MostDetailedMip = resSubresources.BaseMipLevel;
			viewDesc.Texture1DArray.MipLevels = resSubresources.NumMipLevels;
			break;
		case ImageDimension::Image2D:
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MostDetailedMip = resSubresources.BaseMipLevel;
			viewDesc.Texture2D.MipLevels = resSubresources.NumMipLevels;
			viewDesc.Texture2D.PlaneSlice = planeSlice;
			break;
		case ImageDimension::Image2DArray:
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Texture2DArray.FirstArraySlice = resSubresources.BaseArraySlice;
			viewDesc.Texture2DArray.ArraySize = resSubresources.NumArraySlices;
			viewDesc.Texture2DArray.MostDetailedMip = resSubresources.BaseMipLevel;
			viewDesc.Texture2DArray.MipLevels = resSubresources.NumMipLevels;
			viewDesc.Texture2DArray.PlaneSlice = planeSlice;
			break;
		case ImageDimension::ImageCube:
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			viewDesc.TextureCube.MostDetailedMip = resSubresources.BaseMipLevel;
			viewDesc.TextureCube.MipLevels = resSubresources.NumMipLevels;
			break;
		case ImageDimension::ImageCubeArray:
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
			viewDesc.TextureCubeArray.First2DArrayFace = resSubresources.BaseArraySlice;
			viewDesc.TextureCubeArray.NumCubes = resSubresources.NumArraySlices / 6;
			viewDesc.TextureCubeArray.MostDetailedMip = resSubresources.BaseMipLevel;
			viewDesc.TextureCubeArray.MipLevels = resSubresources.NumMipLevels;
			break;
		case ImageDimension::Image2DMS:
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
			break;
		case ImageDimension::Image2DMSArray:
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
			viewDesc.Texture2DMSArray.FirstArraySlice = resSubresources.BaseArraySlice;
			viewDesc.Texture2DMSArray.ArraySize = resSubresources.NumArraySlices;
			break;
		case ImageDimension::Image3D:
			viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			viewDesc.Texture3D.MostDetailedMip = resSubresources.BaseMipLevel;
			viewDesc.Texture3D.MipLevels = resSubresources.NumMipLevels;
			break;

		default:
			NG_UNREACHABLE();
			break;
		}

		// Passthrough to other func
		CreateSRV(index, viewDesc, resource);
	}

	void Dx12DescriptorHeap::CreateSRV(DescriptorHeapIndex index, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, ID3D12Resource* resource)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), "[Dx12DescriptorHeap] Cannot allocate an SRV from a non SRV heap.");
		NG_ASSERT(resource, "[Dx12DescriptorHeap] Resource must not be null.");

		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = GetCPUHandleForIndex(index);
		m_Device.GetContext().GetD3D12Device()->CreateShaderResourceView(resource, &desc, handle);
	}

	void Dx12DescriptorHeap::CreateUAV(DescriptorHeapIndex index, const BufferSpecification& specs, const BufferRange& range, ResourceType type, ID3D12Resource* resource, Format format)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), "[Dx12DescriptorHeap] Cannot allocate an SRV from a non SRV heap.");
		NG_ASSERT(resource, "[Dx12DescriptorHeap] Resource must not be null.");

		BufferRange resRange = ResolveBufferRange(range, specs);

		if (format == Format::Unknown)
			format = specs.BufferFormat;

		FormatInfo info = FormatToFormatInfo(format);
		FormatMapping mapping = FormatToFormatMapping(format);

		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};
		viewDesc.Format = FormatToFormatMapping(format).ResourceFormat;
		viewDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

		switch (type)
		{
		case ResourceType::StructuredBufferSRV:
		{
			NG_ASSERT(specs.Size != 0, "[Dx12DescriptorHeap] Buffer size cannot be equal to 0.");
			NG_ASSERT(specs.Stride != 0, "[Dx12DescriptorHeap] Buffer stride cannot be equal to 0.");

			viewDesc.Format = DXGI_FORMAT_UNKNOWN;
			viewDesc.Buffer.FirstElement = range.Offset / specs.Stride;
			viewDesc.Buffer.NumElements = static_cast<UINT>(range.Size / specs.Stride);
			viewDesc.Buffer.StructureByteStride = static_cast<UINT>(specs.Stride);
			break;
		}

		//case ResourceType::TypedBufferSRV:
		//{
		//	NG_ASSERT(format != Format::Unknown, "[Dx12DescriptorHeap] ...");
		//
		//	viewDesc.Format = mapping.SRVFormat;
		//	viewDesc.Buffer.FirstElement = range.Offset / info.BytesPerBlock;
		//	viewDesc.Buffer.NumElements = static_cast<UINT>(range.Size / info.BytesPerBlock);
		//	break;
		//}

		default:
			NG_UNREACHABLE();
			return;
		}

		// Passthrough to other func
		CreateUAV(index, viewDesc, resource);
	}

	void Dx12DescriptorHeap::CreateUAV(DescriptorHeapIndex index, const ImageSpecification& specs, const ImageSubresourceSpecification& subresources, ID3D12Resource* resource, Format format, ImageDimension dimension)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), "[Dx12DescriptorHeap] Cannot allocate an UAV from a non UAV heap.");
		NG_ASSERT(resource, "[Dx12DescriptorHeap] Resource must not be null.");

		ImageSubresourceSpecification resSubresources = ResolveImageSubresource(subresources, specs, false);

		if (dimension == ImageDimension::Unknown)
			dimension = specs.Dimension;
		if (format == Format::Unknown)
			format = specs.ImageFormat;

		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};
		viewDesc.Format = FormatToFormatMapping(format).SRVFormat;

		switch (specs.Dimension)
		{
		case ImageDimension::Image1D:
			viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
			viewDesc.Texture1D.MipSlice = resSubresources.BaseMipLevel;
			break;
		case ImageDimension::Image1DArray:
			viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
			viewDesc.Texture1DArray.FirstArraySlice = resSubresources.BaseArraySlice;
			viewDesc.Texture1DArray.ArraySize = resSubresources.NumArraySlices;
			viewDesc.Texture1DArray.MipSlice = resSubresources.BaseMipLevel;
			break;
		case ImageDimension::Image2D:
			viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipSlice = resSubresources.BaseMipLevel;
			break;
		case ImageDimension::Image2DArray:
		case ImageDimension::ImageCube:
		case ImageDimension::ImageCubeArray:
			viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Texture2DArray.FirstArraySlice = resSubresources.BaseArraySlice;
			viewDesc.Texture2DArray.ArraySize = resSubresources.NumArraySlices;
			viewDesc.Texture2DArray.MipSlice = resSubresources.BaseMipLevel;
			break;
		case ImageDimension::Image3D:
			viewDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			viewDesc.Texture3D.FirstWSlice = 0;
			viewDesc.Texture3D.WSize = specs.Depth;
			viewDesc.Texture3D.MipSlice = resSubresources.BaseMipLevel;
			break;
		case ImageDimension::Image2DMS:
		case ImageDimension::Image2DMSArray:
		{
			m_Device.GetContext().Error(std::format("Image {0} has unsupported dimension for UAV: Image2DMSArray", specs.DebugName));
			return;
		}

		default:
			NG_UNREACHABLE();
			break;
		}

		// Passthrough to other func
		CreateUAV(index, viewDesc, resource);
	}

	void Dx12DescriptorHeap::CreateUAV(DescriptorHeapIndex index, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, ID3D12Resource* resource)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), "[Dx12DescriptorHeap] Cannot allocate an UAV from a non UAV heap.");
		NG_ASSERT(resource, "[Dx12DescriptorHeap] Resource must not be null.");

		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = GetCPUHandleForIndex(index);
		m_Device.GetContext().GetD3D12Device()->CreateUnorderedAccessView(resource, nullptr, &desc, handle);
	}

	void Dx12DescriptorHeap::CreateCBV(DescriptorHeapIndex index, const BufferSpecification& specs, ResourceType type, ID3D12Resource* resource)
	{
		(void)type;

		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), "[Dx12DescriptorHeap] Cannot allocate a CBV from a non CBV heap.");
		NG_ASSERT(resource, "[Dx12DescriptorHeap] Resource must not be null.");
		NG_ASSERT((specs.Size % BufferSpecification::DefaultUniformBufferAlignment == 0), "Internal Error: [Dx12DescriptorHeap] Constant buffer size must be a aligned to DefaultUniformBufferAlignment.");

		D3D12_CONSTANT_BUFFER_VIEW_DESC viewDesc = {};
		viewDesc.BufferLocation = resource->GetGPUVirtualAddress();
		viewDesc.SizeInBytes = static_cast<UINT>(specs.Size);

		// Passthrough to other func
		CreateCBV(index, viewDesc, resource);
	}

	void Dx12DescriptorHeap::CreateCBV(DescriptorHeapIndex index, const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc, ID3D12Resource* resource)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), "[Dx12DescriptorHeap] Cannot allocate a CBV from a non CBV heap.");
		NG_ASSERT(resource, "[Dx12DescriptorHeap] Resource must not be null.");
		NG_ASSERT((desc.SizeInBytes % BufferSpecification::DefaultUniformBufferAlignment == 0), "Internal Error: [Dx12DescriptorHeap] Constant buffer size must be a aligned to DefaultUniformBufferAlignment.");

		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = GetCPUHandleForIndex(index);
		m_Device.GetContext().GetD3D12Device()->CreateConstantBufferView(&desc, handle);
	}

	void Dx12DescriptorHeap::CreateRTV(DescriptorHeapIndex index, const ImageSpecification& specs, const ImageSubresourceSpecification& subresources, ID3D12Resource* resource, Format format)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV), "[Dx12DescriptorHeap] Cannot allocate an RTV from a non RTV heap.");
		NG_ASSERT(resource, "[Dx12DescriptorHeap] Resource must not be null.");

		ImageSubresourceSpecification resSubresources = ResolveImageSubresource(subresources, specs, false);

		D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
		viewDesc.Format = FormatToFormatMapping(format == Format::Unknown ? specs.ImageFormat : format).RTVFormat;

		switch (specs.Dimension)
		{
		case ImageDimension::Image1D:
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
			viewDesc.Texture1D.MipSlice = resSubresources.BaseMipLevel;
			break;
		case ImageDimension::Image1DArray:
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
			viewDesc.Texture1DArray.FirstArraySlice = resSubresources.BaseArraySlice;
			viewDesc.Texture1DArray.ArraySize = resSubresources.NumArraySlices;
			viewDesc.Texture1DArray.MipSlice = resSubresources.BaseMipLevel;
			break;
		case ImageDimension::Image2D:
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipSlice = resSubresources.BaseMipLevel;
			break;
		case ImageDimension::Image2DArray:
		case ImageDimension::ImageCube:
		case ImageDimension::ImageCubeArray:
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Texture2DArray.ArraySize = resSubresources.NumArraySlices;
			viewDesc.Texture2DArray.FirstArraySlice = resSubresources.BaseArraySlice;
			viewDesc.Texture2DArray.MipSlice = resSubresources.BaseMipLevel;
			break;
		case ImageDimension::Image2DMS:
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
			break;
		case ImageDimension::Image2DMSArray:
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
			viewDesc.Texture2DMSArray.FirstArraySlice = resSubresources.BaseArraySlice;
			viewDesc.Texture2DMSArray.ArraySize = resSubresources.NumArraySlices;
			break;
		case ImageDimension::Image3D:
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
			viewDesc.Texture3D.FirstWSlice = resSubresources.BaseArraySlice;
			viewDesc.Texture3D.WSize = resSubresources.NumArraySlices;
			viewDesc.Texture3D.MipSlice = resSubresources.BaseMipLevel;
			break;

		default:
			NG_UNREACHABLE();
			break;
		}

		// Passthrough to other func
		CreateRTV(index, viewDesc, resource);
	}

	void Dx12DescriptorHeap::CreateRTV(DescriptorHeapIndex index, const D3D12_RENDER_TARGET_VIEW_DESC& desc, ID3D12Resource* resource)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV), "[Dx12DescriptorHeap] Cannot allocate an RTV from a non RTV heap.");
		NG_ASSERT(resource, "[Dx12DescriptorHeap] Resource must not be null.");

		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = GetCPUHandleForIndex(index);
		m_Device.GetContext().GetD3D12Device()->CreateRenderTargetView(resource, &desc, handle);
	}

	void Dx12DescriptorHeap::CreateDSV(DescriptorHeapIndex index, const ImageSpecification& specs, const ImageSubresourceSpecification& subresources, ID3D12Resource* resource, bool isReadOnly)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV), "[Dx12DescriptorHeap] Cannot allocate an DSV from a non DSV heap.");
		NG_ASSERT(resource, "[Dx12DescriptorHeap] Resource must not be null.");

		ImageSubresourceSpecification resSubresources = ResolveImageSubresource(subresources, specs, true);

		D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
		viewDesc.Format = FormatToFormatMapping(specs.ImageFormat).RTVFormat;

		if (isReadOnly)
		{
			viewDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
			if (viewDesc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT || viewDesc.Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
				viewDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		}

		switch (specs.Dimension)
		{
		case ImageDimension::Image1D:
			viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
			viewDesc.Texture1D.MipSlice = resSubresources.BaseMipLevel;
			break;
		case ImageDimension::Image1DArray:
			viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
			viewDesc.Texture1DArray.FirstArraySlice = resSubresources.BaseArraySlice;
			viewDesc.Texture1DArray.ArraySize = resSubresources.NumArraySlices;
			viewDesc.Texture1DArray.MipSlice = resSubresources.BaseMipLevel;
			break;
		case ImageDimension::Image2D:
			viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipSlice = resSubresources.BaseMipLevel;
			break;
		case ImageDimension::Image2DArray:
		case ImageDimension::ImageCube:
		case ImageDimension::ImageCubeArray:
			viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Texture2DArray.ArraySize = resSubresources.NumArraySlices;
			viewDesc.Texture2DArray.FirstArraySlice = resSubresources.BaseArraySlice;
			viewDesc.Texture2DArray.MipSlice = resSubresources.BaseMipLevel;
			break;
		case ImageDimension::Image2DMS:
			viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
			break;
		case ImageDimension::Image2DMSArray:
			viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
			viewDesc.Texture2DMSArray.FirstArraySlice = resSubresources.BaseArraySlice;
			viewDesc.Texture2DMSArray.ArraySize = resSubresources.NumArraySlices;
			break;
		case ImageDimension::Image3D:
		{
			m_Device.GetContext().Error(std::format("Image {0} has unsupported dimension for DSV: ImageDimension::Image3D", specs.DebugName));
			return;
		}

		default:
			NG_UNREACHABLE();
			break;
		}

		// Passthrough to other func
		CreateDSV(index, viewDesc, resource);
	}

	void Dx12DescriptorHeap::CreateDSV(DescriptorHeapIndex index, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc, ID3D12Resource* resource)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV), "[Dx12DescriptorHeap] Cannot allocate an DSV from a non DSV heap.");
		NG_ASSERT(resource, "[Dx12DescriptorHeap] Resource must not be null.");

		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = GetCPUHandleForIndex(index);
		m_Device.GetContext().GetD3D12Device()->CreateDepthStencilView(resource, &desc, handle);
	}

	void Dx12DescriptorHeap::CreateSampler(DescriptorHeapIndex index, const SamplerSpecification& specs)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER), "[Dx12DescriptorHeap] Cannot allocate a Sampler from a non Sampler heap.");

		D3D12_SAMPLER_DESC samplerDesc = {};

		UINT reductionType = static_cast<UINT>(SamplerReductionTypeToD3D12FilterReductionType(specs.ReductionType));
		if (specs.MaxAnisotropy > 1.0f)
			samplerDesc.Filter = D3D12_ENCODE_ANISOTROPIC_FILTER(reductionType);
		else
			samplerDesc.Filter = D3D12_ENCODE_BASIC_FILTER(
				FilterModeToD3D12FilterType(specs.MinFilter),
				FilterModeToD3D12FilterType(specs.MagFilter),
				FilterModeToD3D12FilterType(specs.MipFilter),
				reductionType
			);

		UINT maxAnisotropy = 1u;
		if (specs.MaxAnisotropy == SamplerSpecification::MaxMaxAnisotropyValue)
			maxAnisotropy = 16u;
		else
			maxAnisotropy = std::max(static_cast<UINT>(specs.MaxAnisotropy), 1u);

		samplerDesc.AddressU = SamplerAddresModeToD3D12TextureAddressMode(specs.AddressU);
		samplerDesc.AddressV = SamplerAddresModeToD3D12TextureAddressMode(specs.AddressV);
		samplerDesc.AddressW = SamplerAddresModeToD3D12TextureAddressMode(specs.AddressW);
		samplerDesc.MipLODBias = specs.MipBias;
		samplerDesc.MaxAnisotropy = maxAnisotropy;
		samplerDesc.ComparisonFunc = ((specs.ReductionType == SamplerReductionType::Comparison) ? D3D12_COMPARISON_FUNC_LESS : D3D12_COMPARISON_FUNC_NEVER);
		samplerDesc.BorderColor[0] = specs.BorderColour.r;
		samplerDesc.BorderColor[1] = specs.BorderColour.g;
		samplerDesc.BorderColor[2] = specs.BorderColour.b;
		samplerDesc.BorderColor[3] = specs.BorderColour.a;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

		// Passthrough to other func
		CreateSampler(index, samplerDesc);
	}

	void Dx12DescriptorHeap::CreateSampler(DescriptorHeapIndex index, const D3D12_SAMPLER_DESC& desc)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER), "[Dx12DescriptorHeap] Cannot allocate a Sampler from a non Sampler heap.");
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = GetCPUHandleForIndex(index);
		m_Device.GetContext().GetD3D12Device()->CreateSampler(&desc, handle);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void Dx12DescriptorHeap::Grow(uint32_t minNewSize)
	{
		// Utility function
		auto nextPowerOf2 = [](uint32_t minSize) -> uint32_t
		{
			// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2

			minSize--;
			minSize |= minSize >> 1;
			minSize |= minSize >> 2;
			minSize |= minSize >> 4;
			minSize |= minSize >> 8;
			minSize |= minSize >> 16;
			minSize++;

			return minSize;
		};

		uint32_t oldSize = m_MaxSize;
		uint32_t newSize = nextPowerOf2(minNewSize);

		if constexpr (Information::Validation)
		{
			if (m_IsShaderVisible)
			{
				NG_ASSERT((newSize < ((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) ? static_cast<uint32_t>(D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1) : static_cast<uint32_t>(D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE))), "[Dx12DescriptorHeap] Size of new descriptor heap exceeds maximum size.");
			}
		}

		DxPtr<ID3D12DescriptorHeap> oldHeap = m_DescriptorHeap;
		m_DescriptorHeap = nullptr;

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = Information::FramesInFlight;
		heapDesc.Type = m_Type;
		heapDesc.Flags = (m_IsShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		DX_VERIFY(m_Device.GetContext().GetD3D12Device()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DescriptorHeap)));
		m_CPUStart = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_GPUStart = m_DescriptorHeap->GetGPUDescriptorHandleForHeapStart();

		// Copy old descriptors
		m_Device.GetContext().GetD3D12Device()->CopyDescriptorsSimple(oldSize, m_CPUStart, oldHeap->GetCPUDescriptorHandleForHeapStart(), m_Type);

		m_Device.GetContext().Destroy([heap = oldHeap]() {}); // Note: Holding a reference to the resource is enough to keep it alive (and destroy when the scope ends)
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Getters
	////////////////////////////////////////////////////////////////////////////////////
	CD3DX12_CPU_DESCRIPTOR_HANDLE Dx12DescriptorHeap::GetCPUHandleForIndex(DescriptorHeapIndex index) const
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = m_CPUStart;
		return handle.Offset(index, m_DescriptorSize);
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE Dx12DescriptorHeap::GetGPUHandleForIndex(DescriptorHeapIndex index) const
	{
		NG_ASSERT(m_IsShaderVisible, "[Dx12DescriptorHeap] To be able to retrieve a GPU the heap must be shader visible.");

		CD3DX12_GPU_DESCRIPTOR_HANDLE handle = m_GPUStart;
		return handle.Offset(index, m_DescriptorSize);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
	Dx12DynamicDescriptorHeap::Dx12DynamicDescriptorHeap(const Device& device, uint32_t maxSize, D3D12_DESCRIPTOR_HEAP_TYPE type, bool isShaderVisible, const std::string& debugName)
		: Dx12DescriptorHeap(device, maxSize, type, isShaderVisible, debugName)
	{
		m_FreeEntries.reserve(maxSize);
	}

	Dx12DynamicDescriptorHeap::~Dx12DynamicDescriptorHeap()
	{
		// Note: Dx12DescriptorHeap handles everything
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Creation methods
	////////////////////////////////////////////////////////////////////////////////////
	DescriptorHeapIndex Dx12DynamicDescriptorHeap::CreateSRV(const BufferSpecification& specs, const BufferRange& range, ResourceType type, ID3D12Resource* resource, Format format)
	{
		DescriptorHeapIndex index = GetNextIndex();
		Dx12DescriptorHeap::CreateSRV(index, specs, range, type, resource, format);
		return index;
	}

	DescriptorHeapIndex Dx12DynamicDescriptorHeap::CreateSRV(const ImageSpecification& specs, const ImageSubresourceSpecification& subresources, ID3D12Resource* resource, Format format, ImageDimension dimension)
	{
		DescriptorHeapIndex index = GetNextIndex();
		Dx12DescriptorHeap::CreateSRV(index, specs, subresources, resource, format, dimension);
		return index;
	}

	DescriptorHeapIndex Dx12DynamicDescriptorHeap::CreateSRV(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, ID3D12Resource* resource)
	{
		DescriptorHeapIndex index = GetNextIndex();
		Dx12DescriptorHeap::CreateSRV(index, desc, resource);
		return index;
	}

	DescriptorHeapIndex Dx12DynamicDescriptorHeap::CreateUAV(const BufferSpecification& specs, const BufferRange& range, ResourceType type, ID3D12Resource* resource, Format format)
	{
		DescriptorHeapIndex index = GetNextIndex();
		Dx12DescriptorHeap::CreateUAV(index, specs, range, type, resource, format);
		return index;
	}

	DescriptorHeapIndex Dx12DynamicDescriptorHeap::CreateUAV(const ImageSpecification& specs, const ImageSubresourceSpecification& subresources, ID3D12Resource* resource, Format format, ImageDimension dimension)
	{
		DescriptorHeapIndex index = GetNextIndex();
		Dx12DescriptorHeap::CreateUAV(index, specs, subresources, resource, format, dimension);
		return index;
	}

	DescriptorHeapIndex Dx12DynamicDescriptorHeap::CreateUAV(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, ID3D12Resource* resource)
	{
		DescriptorHeapIndex index = GetNextIndex();
		Dx12DescriptorHeap::CreateUAV(index, desc, resource);
		return index;
	}

	DescriptorHeapIndex Dx12DynamicDescriptorHeap::CreateCBV(const BufferSpecification& specs, ResourceType type, ID3D12Resource* resource)
	{
		DescriptorHeapIndex index = GetNextIndex();
		Dx12DescriptorHeap::CreateCBV(index, specs, type, resource);
		return index;
	}

	DescriptorHeapIndex Dx12DynamicDescriptorHeap::CreateCBV(const D3D12_CONSTANT_BUFFER_VIEW_DESC& desc, ID3D12Resource* resource)
	{
		DescriptorHeapIndex index = GetNextIndex();
		Dx12DescriptorHeap::CreateCBV(index, desc, resource);
		return index;
	}

	DescriptorHeapIndex Dx12DynamicDescriptorHeap::CreateRTV(const ImageSpecification& specs, const ImageSubresourceSpecification& subresources, ID3D12Resource* resource, Format format)
	{
		DescriptorHeapIndex index = GetNextIndex();
		Dx12DescriptorHeap::CreateRTV(index, specs, subresources, resource, format);
		return index;
	}

	DescriptorHeapIndex Dx12DynamicDescriptorHeap::CreateRTV(const D3D12_RENDER_TARGET_VIEW_DESC& desc, ID3D12Resource* resource)
	{
		DescriptorHeapIndex index = GetNextIndex();
		Dx12DescriptorHeap::CreateRTV(index, desc, resource);
		return index;
	}

	DescriptorHeapIndex Dx12DynamicDescriptorHeap::CreateDSV(const ImageSpecification& specs, const ImageSubresourceSpecification& subresources, ID3D12Resource* resource, bool isReadOnly)
	{
		DescriptorHeapIndex index = GetNextIndex();
		Dx12DescriptorHeap::CreateDSV(index, specs, subresources, resource, isReadOnly);
		return index;
	}

	DescriptorHeapIndex Dx12DynamicDescriptorHeap::CreateDSV(const D3D12_DEPTH_STENCIL_VIEW_DESC& desc, ID3D12Resource* resource)
	{
		DescriptorHeapIndex index = GetNextIndex();
		Dx12DescriptorHeap::CreateDSV(index, desc, resource);
		return index;
	}

	DescriptorHeapIndex Dx12DynamicDescriptorHeap::CreateSampler(const SamplerSpecification& specs)
	{
		DescriptorHeapIndex index = GetNextIndex();
		Dx12DescriptorHeap::CreateSampler(index, specs);
		return index;
	}

	DescriptorHeapIndex Dx12DynamicDescriptorHeap::CreateSampler(const D3D12_SAMPLER_DESC& desc)
	{
		DescriptorHeapIndex index = GetNextIndex();
		Dx12DescriptorHeap::CreateSampler(index, desc);
		return index;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Creation methods
	////////////////////////////////////////////////////////////////////////////////////
	void Dx12DynamicDescriptorHeap::Free(DescriptorHeapIndex index)
	{
		// Note: This can definitely be improved, but this is the first/simplest thing
		for (auto& freed : m_FreeEntries)
		{
			if ((freed.Index + freed.Amount) == index)
			{
				freed.Amount += 1;
				return;
			}
			else if ((freed.Index - 1) == index)
			{
				freed.Index = index;
				freed.Amount += 1;
				return;
			}
		}
		
		m_FreeEntries.emplace_back(1, index);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Private methods
	////////////////////////////////////////////////////////////////////////////////////
	DescriptorHeapIndex Dx12DynamicDescriptorHeap::GetNextIndex()
	{
		// If we have a freed slot use that else use the offset
		DescriptorHeapIndex index = {};
		if (!m_FreeEntries.empty())
		{
			Entry& e = m_FreeEntries.back();

			index = e.Index;

			if (e.Amount == 1)
				m_FreeEntries.pop_back();
			else
			{
				e.Amount -= 1;
				e.Index += 1;
			}
		}
		else
		{
			if (m_Count >= m_MaxSize) [[unlikely]]
			{
				m_Device.GetContext().Warn("[Dx12DynamicDescriptorHeap] Grew descriptor heap, this is untested and may cause previous retrieved descriptors to be invalid and crash.");
				Grow(m_Count + 1);
			}

			index = m_Offset;
			m_Offset += 1;
		}

		m_Count++;
		return index;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
	Dx12ManagedDescriptorHeap::Dx12ManagedDescriptorHeap(const Device& device, uint32_t maxSize, D3D12_DESCRIPTOR_HEAP_TYPE type, bool isShaderVisible, const std::string& debugName)
		: Dx12DescriptorHeap(device, maxSize, type, isShaderVisible, debugName)
	{
	}

	Dx12ManagedDescriptorHeap::~Dx12ManagedDescriptorHeap()
	{
		// Note: Dx12DescriptorHeap handles everything
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Getters
	////////////////////////////////////////////////////////////////////////////////////
	DescriptorHeapIndex Dx12ManagedDescriptorHeap::GetNextPoolIndex(uint32_t setCount, uint32_t resourceCount)
	{
		DescriptorHeapIndex index = m_NextPoolIndex;
		m_NextPoolIndex += (setCount * resourceCount);
		return index;
	}

}