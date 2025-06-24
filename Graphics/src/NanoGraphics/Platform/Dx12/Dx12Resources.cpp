#include "ngpch.h"
#include "Dx12Resources.hpp"

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
	Dx12Resources::Heap::Heap(const Device& device, uint32_t maxSize, D3D12_DESCRIPTOR_HEAP_TYPE type, bool isShaderVisible)
		: m_Device(*api_cast<const Dx12Device*>(&device)), m_MaxSize(maxSize), m_Type(type), m_IsShaderVisible(isShaderVisible)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = maxSize;
		heapDesc.Type = m_Type;
		heapDesc.Flags = (m_IsShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
		
		DX_VERIFY(m_Device.GetContext().GetD3D12Device()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DescriptorHeap)));
		
		m_DescriptorSize = m_Device.GetContext().GetD3D12Device()->GetDescriptorHandleIncrementSize(type);
		m_Start = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_Offset = 0;
	}

	Dx12Resources::Heap::~Heap()
	{
		m_Device.GetContext().Destroy([heap = m_DescriptorHeap]() {}); // Note: Holding a reference to the resource is enough to keep it alive (and destroy when the scope ends)
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	Dx12Resources::Heap::Index Dx12Resources::Heap::CreateSRV(Format format, ImageDimension dimension, const ImageSubresourceSpecification& subresources, const ImageSpecification& specs, ID3D12Resource* resource)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), "[Dx12Resources::Heap] Cannot allocate an SRV from a non SRV heap.");
		NG_ASSERT(resource, "[Dx12Resources::Heap] Resource must not be null.");

		ImageSubresourceSpecification resSubresources = ResolveImageSubresouce(subresources, specs, false);

		if (dimension == ImageDimension::Unknown)
			dimension = specs.Dimension;

		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.Format = FormatToFormatMapping(format == Format::Unknown ? specs.ImageFormat : format).SRVFormat;
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		uint32_t planeSlice = (viewDesc.Format == DXGI_FORMAT_X24_TYPELESS_G8_UINT) ? 1 : 0;

		Index index = GetNextIndex();
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = GetHandleForIndex(index);

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

		m_Device.GetContext().GetD3D12Device()->CreateShaderResourceView(resource, &viewDesc, handle);
		return index;
	}

	Dx12Resources::Heap::Index Dx12Resources::Heap::CreateUAV(Format format, ImageDimension dimension, const ImageSubresourceSpecification& subresources, const ImageSpecification& specs, ID3D12Resource* resource)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), "[Dx12Resources::Heap] Cannot allocate an UAV from a non UAV heap.");
		NG_ASSERT(resource, "[Dx12Resources::Heap] Resource must not be null.");

		ImageSubresourceSpecification resSubresources = ResolveImageSubresouce(subresources, specs, false);

		if (dimension == ImageDimension::Unknown)
			dimension = specs.Dimension;

		D3D12_UNORDERED_ACCESS_VIEW_DESC viewDesc = {};
		viewDesc.Format = FormatToFormatMapping(format == Format::Unknown ? specs.ImageFormat : format).SRVFormat;

		Index index = GetNextIndex();
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = GetHandleForIndex(index);

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
			return index;
		}

		default:
			NG_UNREACHABLE();
			break;
		}

		m_Device.GetContext().GetD3D12Device()->CreateUnorderedAccessView(resource, nullptr, &viewDesc, handle);
		return index;
	}

	Dx12Resources::Heap::Index Dx12Resources::Heap::CreateRTV(Format format, const ImageSubresourceSpecification& subresources, const ImageSpecification& specs, ID3D12Resource* resource)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV), "[Dx12Resources::Heap] Cannot allocate an RTV from a non RTV heap.");
		NG_ASSERT(resource, "[Dx12Resources::Heap] Resource must not be null.");

		ImageSubresourceSpecification resSubresources = ResolveImageSubresouce(subresources, specs, false);

		D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
		viewDesc.Format = FormatToFormatMapping(format == Format::Unknown ? specs.ImageFormat : format).RTVFormat;

		Index index = GetNextIndex();
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = GetHandleForIndex(index);

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

		m_Device.GetContext().GetD3D12Device()->CreateRenderTargetView(resource, &viewDesc, handle);
		return index;
	}

	Dx12Resources::Heap::Index Dx12Resources::Heap::CreateDSV(const ImageSubresourceSpecification& subresources, const ImageSpecification& specs, ID3D12Resource* resource, bool isReadOnly)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV), "[Dx12Resources::Heap] Cannot allocate an DSV from a non DSV heap.");
		NG_ASSERT(resource, "[Dx12Resources::Heap] Resource must not be null.");

		ImageSubresourceSpecification resSubresources = ResolveImageSubresouce(subresources, specs, true);

		D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
		viewDesc.Format = FormatToFormatMapping(specs.ImageFormat).RTVFormat;

		if (isReadOnly)
		{
			viewDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
			if (viewDesc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT || viewDesc.Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
				viewDesc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		}

		Index index = GetNextIndex();
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = GetHandleForIndex(index);

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
			return index;
		}

		default:
			NG_UNREACHABLE();
			break;
		}

		m_Device.GetContext().GetD3D12Device()->CreateDepthStencilView(resource, &viewDesc, handle);
		return index;
	}

	Dx12Resources::Heap::Index Dx12Resources::Heap::CreateSampler(const D3D12_SAMPLER_DESC& desc)
	{
		NG_ASSERT((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER), "[Dx12Resources::Heap] Cannot allocate a Sampler from a non Sampler heap.");

		Index index = GetNextIndex();
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = GetHandleForIndex(index);

		m_Device.GetContext().GetD3D12Device()->CreateSampler(&desc, handle);
		return index;
	}

	void Dx12Resources::Heap::Free(Index handle)
	{
		m_FreeEntries.emplace_back(1, handle);
	}

	void Dx12Resources::Heap::Grow(uint32_t minNewSize)
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
				NG_ASSERT((newSize < ((m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) ? static_cast<uint32_t>(D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1) : static_cast<uint32_t>(D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE))), "[Dx12Resources::Heap] Size of new descriptor heap exceeds maximum size.");
			}
		}

		DxPtr<ID3D12DescriptorHeap> oldHeap = m_DescriptorHeap;
		m_DescriptorHeap = nullptr;

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = Information::FramesInFlight;
		heapDesc.Type = m_Type;
		heapDesc.Flags = (m_IsShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE);

		DX_VERIFY(m_Device.GetContext().GetD3D12Device()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_DescriptorHeap)));
		m_Start = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_Offset = 0;

		// Copy old descriptors
		m_Device.GetContext().GetD3D12Device()->CopyDescriptorsSimple(oldSize, m_Start, oldHeap->GetCPUDescriptorHandleForHeapStart(), m_Type);

		// Note: The previous FreeEntries are still valid since we just store an index

		m_Device.GetContext().Destroy([heap = oldHeap]() {}); // Note: Holding a reference to the resource is enough to keep it alive (and destroy when the scope ends)
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Getters
	////////////////////////////////////////////////////////////////////////////////////
	CD3DX12_CPU_DESCRIPTOR_HANDLE Dx12Resources::Heap::GetHandleForIndex(Index index) const
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE handle = m_Start;
		return handle.Offset(index, m_DescriptorSize);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Private methods
	////////////////////////////////////////////////////////////////////////////////////
	Dx12Resources::Heap::Index Dx12Resources::Heap::GetNextIndex()
	{
		// If we have a freed slot use that else use the offset
		Index index = {};
		if (!m_FreeEntries.empty())
		{
			Entry& e = m_FreeEntries.back();

			index = e.IndexForHandle;

			if (e.Amount == 1)
				m_FreeEntries.pop_back();
			else
			{
				e.Amount -= 1;
				e.IndexForHandle += 1;
			}
		}
		else
		{
			if (m_Count >= m_MaxSize) [[unlikely]]
			{
				m_Device.GetContext().Warn("[Dx12Resources::Heap] Grew descriptor heap, this is untested and may cause previous retrieved descriptors to be invalid and crash.");
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
	Dx12Resources::Dx12Resources(const Device& device)
		: m_Device(*api_cast<const Dx12Device*>(&device)), m_SRVAndUAVHeap(device, s_SRVAndUAVStartSize, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true), m_SamplerHeap(device, s_SamplerStartSize, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, true), m_DSVHeap(device, s_DSVStartSize, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, false), m_RTVHeap(device, s_RTVStartSize, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false)
	{
	}

	Dx12Resources::~Dx12Resources()
	{
		// Note: The destructor of the Heaps will release the resources
	}

}