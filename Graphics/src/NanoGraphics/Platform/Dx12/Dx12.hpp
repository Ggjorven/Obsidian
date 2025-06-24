#pragma once

#if defined(NG_API_DX12)
#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Information.hpp"

#include <Nano/Nano.hpp>

#include <Windows.h>

#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <D3D12MemAlloc.h>

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Helper macro
    ////////////////////////////////////////////////////////////////////////////////////
    #define DX_VERIFY_IMPL2(expr, str, num)                                                                             \
                HRESULT result##num = expr;                                                                             \
                if (result##num < 0)                                                                                    \
                NG_LOG_ERROR("Expression '{0}' failed with error: {1} ({2})", str, ::Nano::Graphics::Internal::HRESULTToDescription(result##num), ::Nano::Graphics::Internal::HRESULTToString(result##num))
    #define DX_VERIFY_IMPL(expr, str, num) DX_VERIFY_IMPL2(expr, str, num)

    #if !defined(NG_CONFIG_DIST)
        #define DX_VERIFY(expr) DX_VERIFY_IMPL((expr), #expr, __COUNTER__)
    #else
        #define DX_VERIFY(expr) expr
    #endif

    #define DX_SUCCESS(expr) SUCCEEDED(expr)
    #define DX_FAILED(expr) FAILED(expr)

    ////////////////////////////////////////////////////////////////////////////////////
    // ComPtr wrapper
    ////////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    using DxPtr = Microsoft::WRL::ComPtr<T>;

    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12Allocator
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12Allocator
    {
    public:
        // Constructor & Destructor
        Dx12Allocator(IDXGIAdapter1* adapter, DxPtr<ID3D12Device> device);
        ~Dx12Allocator();

        // Buffers
        DxPtr<D3D12MA::Allocation> AllocateBuffer(DxPtr<ID3D12Resource>& resource, size_t size, D3D12_RESOURCE_STATES initialState, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT) const;
        DxPtr<D3D12MA::Allocation> AllocateBuffer(DxPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES initialState, D3D12_RESOURCE_DESC resourceDesc, D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT) const;
        DxPtr<D3D12MA::Allocation> AllocateBuffer(DxPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES initialState, D3D12_RESOURCE_DESC resourceDesc, D3D12MA::ALLOCATION_DESC allocationDesc) const;

        // Image
        DxPtr<D3D12MA::Allocation> CreateImage(DxPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES initialState, D3D12_RESOURCE_DIMENSION dimension, uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t mipLevels, DXGI_FORMAT format, uint32_t sampleCount, uint32_t sampleQuality, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT) const;
        DxPtr<D3D12MA::Allocation> CreateImage(DxPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES initialState, D3D12_RESOURCE_DESC resourceDesc, D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT) const;
        DxPtr<D3D12MA::Allocation> CreateImage(DxPtr<ID3D12Resource>& resource, D3D12_RESOURCE_STATES initialState, D3D12_RESOURCE_DESC resourceDesc, D3D12MA::ALLOCATION_DESC allocationDesc) const;

        // Utils
        //void MapMemory(VmaAllocation allocation, void*& mapData) const;
        //void UnmapMemory(VmaAllocation allocation) const;
        //void SetData(VmaAllocation allocation, void* data, size_t size) const;
        //void SetMappedData(void* mappedData, void* data, size_t size) const;

        inline static const D3D12MA::ALLOCATION_CALLBACKS* GetCallbacks() { return &s_Callbacks; }

    private:
        DxPtr<ID3D12Device> m_Device = nullptr;

        DxPtr<D3D12MA::Allocator> m_Allocator = nullptr;

        inline static D3D12MA::ALLOCATION_CALLBACKS s_Callbacks = {};
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ToString methods
    ////////////////////////////////////////////////////////////////////////////////////
    inline std::string HRESULTToDescription(HRESULT hr) 
    {
        char* msgBuffer = nullptr;
        DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

        (void)FormatMessageA(
            flags,
            nullptr,
            hr,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPSTR>(&msgBuffer),
            0,
            nullptr);

        std::string result = msgBuffer ? msgBuffer : "Unknown HRESULT";
        LocalFree(msgBuffer); // Free the buffer allocated by FormatMessage
        return result;
    }

    inline constexpr std::string_view HRESULTToString(HRESULT result)
    {
        switch (result) 
        {
        // Success codes
        case S_OK: return "S_OK";
        case S_FALSE: return "S_FALSE";

        // Generic error codes
        case E_ABORT: return "E_ABORT";
        case E_ACCESSDENIED: return "E_ACCESSDENIED";
        case E_FAIL: return "E_FAIL";
        case E_HANDLE: return "E_HANDLE";
        case E_INVALIDARG: return "E_INVALIDARG";
        case E_NOINTERFACE: return "E_NOINTERFACE";
        case E_NOTIMPL: return "E_NOTIMPL";
        case E_OUTOFMEMORY: return "E_OUTOFMEMORY";
        case E_POINTER: return "E_POINTER";
        case E_UNEXPECTED: return "E_UNEXPECTED";

        // DXGI (DirectX Graphics Infrastructure)
        case DXGI_ERROR_DEVICE_HUNG: return "DXGI_ERROR_DEVICE_HUNG";
        case DXGI_ERROR_DEVICE_REMOVED: return "DXGI_ERROR_DEVICE_REMOVED";
        case DXGI_ERROR_DEVICE_RESET: return "DXGI_ERROR_DEVICE_RESET";
        case DXGI_ERROR_DRIVER_INTERNAL_ERROR: return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
        case DXGI_ERROR_FRAME_STATISTICS_DISJOINT: return "DXGI_ERROR_FRAME_STATISTICS_DISJOINT";
        case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: return "DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE";
        case DXGI_ERROR_INVALID_CALL: return "DXGI_ERROR_INVALID_CALL";
        case DXGI_ERROR_MORE_DATA: return "DXGI_ERROR_MORE_DATA";
        case DXGI_ERROR_NONEXCLUSIVE: return "DXGI_ERROR_NONEXCLUSIVE";
        case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE: return "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE";
        case DXGI_ERROR_NOT_FOUND: return "DXGI_ERROR_NOT_FOUND";
        case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED: return "DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED";
        case DXGI_ERROR_REMOTE_OUTOFMEMORY: return "DXGI_ERROR_REMOTE_OUTOFMEMORY";
        case DXGI_ERROR_WAS_STILL_DRAWING: return "DXGI_ERROR_WAS_STILL_DRAWING";

        // D3D12 (Direct3D 12 specific HRESULTs)
        case D3D12_ERROR_ADAPTER_NOT_FOUND: return "D3D12_ERROR_ADAPTER_NOT_FOUND";
        case D3D12_ERROR_DRIVER_VERSION_MISMATCH: return "D3D12_ERROR_DRIVER_VERSION_MISMATCH";

        default:
            break;
        }

        return "Unknown HRESULT";
    }

}
#endif