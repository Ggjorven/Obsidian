#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/DeviceSpec.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"

#include <Nano/Nano.hpp>

#include <tuple>
#include <string>

namespace Nano::Graphics::Internal
{

    class Dx12Context;

#if defined(NG_API_DX12)
    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12Context
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12Context
    {
    public:
        inline constexpr static const std::tuple<uint8_t, uint8_t> Version = { 12, 1 };
    public:
        // Constructors & Destructor
        Dx12Context(DeviceMessageCallback messageCallback, DeviceDestroyCallback destroyCallback);
        ~Dx12Context();

        // Methods
        void Warn(const std::string& message);
        void Error(const std::string& message);

        void Destroy(DeviceDestroyFn fn);

        // Internal methods
        void InitMessageQueue(ID3D12Device* device);

        void SetDebugName(ID3D12Object* object, const std::wstring& name);

    private:
        ID3D12Debug* m_DebugController = nullptr;
        ID3D12InfoQueue* m_MessageQueue = nullptr;
        
        DeviceDestroyCallback m_DestroyCallback;
    };
#endif

}