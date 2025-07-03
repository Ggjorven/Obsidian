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
		: m_Device(*api_cast<const Dx12Device*>(&device)), m_SRVAndUAVAndCBVHeap(device, s_SRVAndUAVAndCBVStartSize, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true, "SRV, UAV & CBV DescriptorHeap"), m_SamplerHeap(device, s_SamplerStartSize, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, true, "Sampler DescriptorHeap"), m_DSVHeap(device, s_DSVStartSize, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, false, "DSV DescriptorHeap"), m_RTVHeap(device, s_RTVStartSize, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false, "RTV DescriptorHeap")
	{
	}

	Dx12Resources::~Dx12Resources()
	{
		// Note: The destructor of the Heaps will release the resources
	}

    ////////////////////////////////////////////////////////////////////////////////////
    // Other
    ////////////////////////////////////////////////////////////////////////////////////
	uint8_t Dx12FormatToPlaneCount(const Device& device, DXGI_FORMAT format)
    {
        const Dx12Device& dxDevice = *api_cast<const Dx12Device*>(&device);

        uint8_t planeCount = 0;

        D3D12_FEATURE_DATA_FORMAT_INFO formatInfo = { format, 1 };
        if (DX_FAILED(dxDevice.GetContext().GetD3D12Device()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO, &formatInfo, sizeof(formatInfo))))
            // Format not supported
            planeCount = 255;
        else
            // Format supported
            planeCount = formatInfo.PlaneCount;

        return planeCount;
    }

}