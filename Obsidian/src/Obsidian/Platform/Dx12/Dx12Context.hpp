#pragma once

#include "Obsidian/Core/Information.hpp"
#include "Obsidian/Core/Logging.hpp"

#include "Obsidian/Renderer/DeviceSpec.hpp"
#include "Obsidian/Renderer/CommandListSpec.hpp"

#include "Obsidian/Platform/Dx12/Dx12.hpp"

#include <Nano/Nano.hpp>

#include <tuple>
#include <string>

namespace Obsidian::Internal
{

    class Dx12Context;

#if defined(OB_API_DX12)
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
        void Warn(const std::string& message) const;
        void Error(const std::string& message) const;
        void OutputMessages() const;

        void Destroy(DeviceDestroyFn fn) const;

        // Internal methods
        void SetDebugName(ID3D12Object* object, const std::string& name) const;
        void SetDebugName(ID3D12Object* object, const std::wstring& name) const;

        // Internal getters
        bool AllowsTearing() const;

        inline DxPtr<IDXGIAdapter1> GetD3D12Adapter() const { return m_Adapter; }
        inline DxPtr<ID3D12Device> GetD3D12Device() const { return m_Device; }
        inline DxPtr<IDXGIFactory7> GetIDXGIFactory() const { return m_Factory; }
        inline DxPtr<ID3D12CommandQueue> GetD3D12CommandQueue(CommandQueue queue) const { OB_ASSERT((static_cast<size_t>(queue) < static_cast<size_t>(CommandQueue::Count)), "[Dx12Context] Invalid CommandQueue passed in."); return m_Queues[static_cast<size_t>(queue)]; }

        inline const std::array<DxPtr<ID3D12CommandQueue>, static_cast<size_t>(CommandQueue::Count)>& GetD3D12CommandQueues() const { return m_Queues; }

    private:
        DeviceDestroyCallback m_DestroyCallback;

        DxPtr<IDXGIFactory7> m_Factory = nullptr;
        DxPtr<IDXGIAdapter1> m_Adapter = nullptr; // Physical device
        DxPtr<ID3D12Device> m_Device = nullptr; // Logical device

        DxPtr<ID3D12Debug6> m_DebugController = nullptr;
        DxPtr<ID3D12InfoQueue> m_MessageQueue = nullptr;

        std::array<DxPtr<ID3D12CommandQueue>, static_cast<size_t>(CommandQueue::Count)> m_Queues = {};
    };
#endif

}