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
	Dx12Resources::Dx12Resources(const Device& device)
		: m_Device(*api_cast<const Dx12Device*>(&device)), m_SRVAndUAVHeap(device, s_SRVAndUAVStartSize, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true), m_SamplerHeap(device, s_SamplerStartSize, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, true), m_DSVHeap(device, s_DSVStartSize, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, false), m_RTVHeap(device, s_RTVStartSize, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false)
	{
	}

	Dx12Resources::~Dx12Resources()
	{
		// Note: The destructor of the Heaps will release the resources
	}

}