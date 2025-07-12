#include "ngpch.h"
#include "Dx12.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

namespace Nano::Graphics::Internal
{

    using namespace D3D12MA;

    namespace
    {

        ////////////////////////////////////////////////////////////////////////////////////
        // Callback functions
        ////////////////////////////////////////////////////////////////////////////////////
        void* AllocateCallback(size_t size, size_t alignment, void* pPrivateData)
        {
            (void)alignment; (void)pPrivateData;
            return std::malloc(size);
        }

        void FreeCallback(void* pMemory, void* pPrivateData)
        {
            (void)pPrivateData;
            std::free(pMemory);
        }

    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12Allocator::Dx12Allocator(IDXGIAdapter1* adapter, DxPtr<ID3D12Device> device)
    {
        s_Callbacks.pAllocate = &AllocateCallback;
        s_Callbacks.pFree = &FreeCallback;
        s_Callbacks.pPrivateData = nullptr;

        ALLOCATOR_DESC allocatorDesc = {};
        allocatorDesc.Flags = ALLOCATOR_FLAG_NONE;
        allocatorDesc.pDevice = device.Get();
        allocatorDesc.PreferredBlockSize = 0; // 64 MiB
        allocatorDesc.pAdapter = adapter;
        allocatorDesc.pAllocationCallbacks = &s_Callbacks;

        DX_VERIFY(CreateAllocator(&allocatorDesc, &m_Allocator));
    }

    Dx12Allocator::~Dx12Allocator()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Buffer methodss
    ////////////////////////////////////////////////////////////////////////////////////
    DxPtr<Allocation> Dx12Allocator::AllocateBuffer(DxPtr<ID3D12Resource>& resource, size_t size, D3D12_RESOURCE_STATES initialState, D3D12_RESOURCE_FLAGS flags, D3D12_HEAP_TYPE heapType) const
    {
        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        resourceDesc.Width = size;
        resourceDesc.Height = 1;
        resourceDesc.DepthOrArraySize = 1;
        resourceDesc.MipLevels = 1;
        resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        resourceDesc.SampleDesc.Count = 1;
        resourceDesc.SampleDesc.Quality = 0;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDesc.Flags = flags;

        ALLOCATION_DESC allocDesc = {};
        allocDesc.Flags = ALLOCATION_FLAG_NONE;
        allocDesc.HeapType = heapType;
        allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;
        allocDesc.CustomPool = nullptr;
        allocDesc.pPrivateData = nullptr;

        return AllocateBuffer(resource, initialState, resourceDesc, allocDesc);
    }

    DxPtr<D3D12MA::Allocation> Dx12Allocator::AllocateBuffer(DxPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES initialState, D3D12_RESOURCE_DESC resourceDesc, D3D12_HEAP_TYPE heapType) const
    {
        ALLOCATION_DESC allocDesc = {};
        allocDesc.Flags = ALLOCATION_FLAG_NONE;
        allocDesc.HeapType = heapType;
        allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;
        allocDesc.CustomPool = nullptr;
        allocDesc.pPrivateData = nullptr;

        return AllocateBuffer(resource, initialState, resourceDesc, allocDesc);
    }

    DxPtr<D3D12MA::Allocation> Dx12Allocator::AllocateBuffer(DxPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES initialState, D3D12_RESOURCE_DESC resourceDesc, D3D12MA::ALLOCATION_DESC allocationDesc) const
    {
        DxPtr<Allocation> allocation;
        DX_VERIFY(m_Allocator->CreateResource(&allocationDesc, &resourceDesc, initialState, nullptr, &allocation, IID_PPV_ARGS(&resource)));

        return allocation;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Image methods
    ////////////////////////////////////////////////////////////////////////////////////
    DxPtr<Allocation> Dx12Allocator::CreateImage(DxPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES initialState, D3D12_RESOURCE_DIMENSION dimension, uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t mipLevels, DXGI_FORMAT format, uint32_t sampleCount, uint32_t sampleQuality, D3D12_RESOURCE_FLAGS flags, D3D12_HEAP_TYPE heapType) const
    {
        D3D12_RESOURCE_DESC resourceDesc = {};
        resourceDesc.Dimension = dimension;
        resourceDesc.Alignment = 0ull;
        resourceDesc.Width = width;
        resourceDesc.Height = height;
        resourceDesc.DepthOrArraySize = static_cast<UINT16>(depthOrArraySize);
        resourceDesc.MipLevels = static_cast<UINT16>(mipLevels);
        resourceDesc.Format = format;
        resourceDesc.SampleDesc.Count = sampleCount;
        resourceDesc.SampleDesc.Quality = sampleQuality;
        resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDesc.Flags = flags;

        ALLOCATION_DESC allocDesc = {};
        allocDesc.Flags = ALLOCATION_FLAG_NONE;
        allocDesc.HeapType = heapType;
        allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;
        allocDesc.CustomPool = nullptr;
        allocDesc.pPrivateData = nullptr;

        return CreateImage(resource, initialState, resourceDesc, allocDesc);
    }

    DxPtr<D3D12MA::Allocation> Dx12Allocator::CreateImage(DxPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES initialState, D3D12_RESOURCE_DESC resourceDesc, D3D12_HEAP_TYPE heapType) const
    {
        ALLOCATION_DESC allocDesc = {};
        allocDesc.Flags = ALLOCATION_FLAG_NONE;
        allocDesc.HeapType = heapType;
        allocDesc.ExtraHeapFlags = D3D12_HEAP_FLAG_NONE;
        allocDesc.CustomPool = nullptr;
        allocDesc.pPrivateData = nullptr;

        return CreateImage(resource, initialState, resourceDesc, allocDesc);
    }

    DxPtr<D3D12MA::Allocation> Dx12Allocator::CreateImage(DxPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES initialState, D3D12_RESOURCE_DESC resourceDesc, D3D12MA::ALLOCATION_DESC allocationDesc) const
    {
        DxPtr<Allocation> allocation;
        DX_VERIFY(m_Allocator->CreateResource(&allocationDesc, &resourceDesc, initialState, nullptr, &allocation, IID_PPV_ARGS(&resource)));

        return allocation;
    }

}