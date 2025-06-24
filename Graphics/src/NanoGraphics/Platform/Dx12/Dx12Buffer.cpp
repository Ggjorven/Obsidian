#include "ngpch.h"
#include "Dx12Buffer.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12Device.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Resources.hpp"

namespace Nano::Graphics::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
	Dx12Buffer::Dx12Buffer(const Device& device, const BufferSpecification& specs)
		: m_Specification(specs)
	{
        const Dx12Device& dxDevice = *api_cast<const Dx12Device*>(&device);

        size_t size = m_Specification.Size;

        if (m_Specification.IsUnorderedAccessed) // Storage buffer
            size = Nano::Memory::AlignOffset(size, 256ull);

        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
        if (m_Specification.IsUnorderedAccessed)
            flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        //D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;
        D3D12_RESOURCE_STATES initialState = ResourceStateToD3D12ResourceStates(m_Specification.PermanentState);
        D3D12_HEAP_TYPE heapType;

        switch (m_Specification.CpuAccess)
        {
        case CpuAccessMode::None:
            heapType = D3D12_HEAP_TYPE_DEFAULT;
            break;
        case CpuAccessMode::Read:
            heapType = D3D12_HEAP_TYPE_READBACK;
            break;
        case CpuAccessMode::Write:
            heapType = D3D12_HEAP_TYPE_UPLOAD;
            break;

        default:
            NG_UNREACHABLE();
            break;
        }

        m_Allocation = dxDevice.GetAllocator().AllocateBuffer(m_Resource,
            size, initialState,
            flags, heapType
        );
	}

	Dx12Buffer::~Dx12Buffer()
	{
	}

}