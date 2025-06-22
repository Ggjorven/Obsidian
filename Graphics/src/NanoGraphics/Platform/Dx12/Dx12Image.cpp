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
	Dx12ImageSubresourceView::Dx12ImageSubresourceView(const Image& image, const ImageSubresourceSpecification& specs, CD3DX12_CPU_DESCRIPTOR_HANDLE handle)
		: m_Image(*api_cast<const Dx12Image*>(&image)), m_Specification(specs), m_Handle(handle)
	{
	}

	Dx12ImageSubresourceView::~Dx12ImageSubresourceView()
	{
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
	void Dx12Image::SetInternalData(const ImageSpecification& specs, ID3D12Resource* image)
	{
		m_Specification = specs;
		m_Resource = image;
	}

	void Dx12Image::SetInternalData(const ImageSpecification& specs, ID3D12Resource* image, ID3D12DescriptorHeap* heap, CD3DX12_CPU_DESCRIPTOR_HANDLE offset)
	{
		m_Specification = specs;
		m_Resource = image;

		m_RTVHeap = { heap, offset };
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Internal getters
	////////////////////////////////////////////////////////////////////////////////////
	const Dx12ImageSubresourceView& Dx12Image::GetSubresourceView(const ImageSubresourceSpecification& specs, ImageSubresourceViewUsage usage, ImageDimension dimension, Format format)
	{
		// TODO: ...
		return *static_cast<const Dx12ImageSubresourceView*>(nullptr);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Private methods
	////////////////////////////////////////////////////////////////////////////////////
	void Dx12Image::CreateSRV(Format format, ImageDimension dimension, const ImageSubresourceSpecification& subresources) const
	{
		// TODO: ...
	}

	void Dx12Image::CreateUAV(Format format, ImageDimension dimension, const ImageSubresourceSpecification& subresources) const
	{
		// TODO: ...
	}

	void Dx12Image::CreateRTV(Format format, const ImageSubresourceSpecification& subresources) const
	{
		// TODO: ...
	}

	void Dx12Image::CreateDSV(const ImageSubresourceSpecification& subresources, bool isReadOnly) const
	{
		// TODO: ...
	}

}