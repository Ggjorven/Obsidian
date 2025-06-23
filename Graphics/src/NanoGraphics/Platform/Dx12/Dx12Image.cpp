#include "ngpch.h"
#include "Dx12Swapchain.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12Device.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Resources.hpp"
#include "Dx12Image.hpp"

namespace Nano::Graphics::Internal
{

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
		// TODO: ...
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
		{
			imageView.m_Index = m_Device.GetResources().GetSRVAndUAVHeap().CreateSRV(format, dimension, specs, m_Specification, m_Resource.Get());
			break;
		}
		case ImageSubresourceViewUsage::UAV:
		{
			imageView.m_Index = m_Device.GetResources().GetSRVAndUAVHeap().CreateUAV(format, dimension, specs, m_Specification, m_Resource.Get());
			break;
		}
		case ImageSubresourceViewUsage::RTV:
		{
			imageView.m_Index = m_Device.GetResources().GetRTVHeap().CreateRTV(format, specs, m_Specification, m_Resource.Get());
			break;
		}
		case ImageSubresourceViewUsage::DSV:
		{
			imageView.m_Index = m_Device.GetResources().GetDSVHeap().CreateDSV(specs, m_Specification, m_Resource.Get());
			break;
		}

		default:
			break;
		}

		return imageView;
	}

}