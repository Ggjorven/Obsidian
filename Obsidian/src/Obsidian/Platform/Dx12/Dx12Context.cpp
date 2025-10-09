#include "obpch.h"
#include "Dx12Context.hpp"

#include "Obsidian/Core/Logging.hpp"
#include "Obsidian/Core/Information.hpp"
#include "Obsidian/Utils/Profiler.hpp"

namespace Obsidian::Internal
{

    namespace
    {

        ////////////////////////////////////////////////////////////////////////////////////
        // Static callback/functions
        ////////////////////////////////////////////////////////////////////////////////////
        static Obsidian::DeviceMessageCallback s_MessageCallback = {};

        ////////////////////////////////////////////////////////////////////////////////////
        // Conversion functions
        ////////////////////////////////////////////////////////////////////////////////////
        static constexpr D3D_FEATURE_LEVEL GetDx12FeatureLevel()
        {
            NANO_ASSERT((std::get<0>(Dx12Context::Version) == 12), "[Dx12Device] Dx12Version must start with 12.");

            switch (std::get<1>(Dx12Context::Version))
            {
            case 0:         return D3D_FEATURE_LEVEL_12_0;
            case 1:         return D3D_FEATURE_LEVEL_12_1;
            case 2:         return D3D_FEATURE_LEVEL_12_2;

            default:
                break;
            }

            return D3D_FEATURE_LEVEL_12_0;
        }

        ////////////////////////////////////////////////////////////////////////////////////
        // Validation method
        ////////////////////////////////////////////////////////////////////////////////////
        void IterateD3D12Messages(ID3D12InfoQueue* infoQueue) 
        {
            const UINT64 messageCount = infoQueue->GetNumStoredMessages();

            for (UINT64 i = 0; i < messageCount; i++) 
            {
                SIZE_T messageLength = 0;
                infoQueue->GetMessage(i, nullptr, &messageLength);

                std::vector<char> buffer(messageLength);
                auto* dxMessage = reinterpret_cast<D3D12_MESSAGE*>(buffer.data());
                infoQueue->GetMessage(i, dxMessage, &messageLength);

                DeviceMessageType messageType = DeviceMessageType::Trace;
                switch (dxMessage->Severity)
                {
                case D3D12_MESSAGE_SEVERITY_CORRUPTION: messageType = DeviceMessageType::Error;     break;
                case D3D12_MESSAGE_SEVERITY_ERROR:      messageType = DeviceMessageType::Error;     break;
                case D3D12_MESSAGE_SEVERITY_WARNING:    messageType = DeviceMessageType::Warn;      break;
                case D3D12_MESSAGE_SEVERITY_INFO:       messageType = DeviceMessageType::Info;      break;
                case D3D12_MESSAGE_SEVERITY_MESSAGE:    messageType = DeviceMessageType::Trace;     break;

                default:
                    OB_UNREACHABLE();
                    break;
                }

                s_MessageCallback(messageType, std::string(dxMessage->pDescription));
            }

            infoQueue->ClearStoredMessages();
        }

    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12Context::Dx12Context(DeviceMessageCallback messageCallback, DeviceDestroyCallback destroyCallback)
        : m_DestroyCallback(destroyCallback)
    {
        s_MessageCallback = messageCallback;

        UINT factoryFlags = 0;

        // Debug interface
        {
            if constexpr (Information::Validation)
            {
                DxPtr<ID3D12Debug> debug;
                DX_VERIFY(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)));

                DX_VERIFY(debug->QueryInterface(IID_PPV_ARGS(&m_DebugController)));
                m_DebugController->EnableDebugLayer();
                m_DebugController->SetEnableGPUBasedValidation(TRUE);
                m_DebugController->SetEnableSynchronizedCommandQueueValidation(TRUE);

                //m_GPUDebugController->ReportLiveObjects();
                factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }

            OB_ASSERT(m_DebugController, "[Dx12Context] Failed to create debug controller.");
        }

        // Physical device
        {
            DX_VERIFY(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_Factory)));
            
            for (UINT i = 0; m_Factory->EnumAdapters1(i, &m_Adapter) != DXGI_ERROR_NOT_FOUND; i++) 
            {
                DXGI_ADAPTER_DESC1 desc;
                DX_VERIFY(m_Adapter->GetDesc1(&desc));

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) 
                    continue;

                if (DX_SUCCESS(D3D12CreateDevice(m_Adapter.Get(), GetDx12FeatureLevel(), __uuidof(ID3D12Device), nullptr)))
                    break;
            }

            OB_ASSERT(m_Adapter, "[Dx12Context] Failed to select physical device.");
        }

        // Logical device
        {
            DX_VERIFY(D3D12CreateDevice(m_Adapter.Get(), GetDx12FeatureLevel(), IID_PPV_ARGS(&m_Device)));

            OB_ASSERT(m_Device, "[Dx12Context] Failed to create logical device.");
        }

        // Queues 
        {
            D3D12_COMMAND_QUEUE_DESC queueDesc = {};
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            queueDesc.NodeMask = 0;

            DX_VERIFY(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_Queues[static_cast<size_t>(CommandQueue::Graphics)])));

            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            DX_VERIFY(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_Queues[static_cast<size_t>(CommandQueue::Compute)])));

            // Note: DX12 doesn't really have a Present queue, so we use the graphics queue
            m_Queues[static_cast<size_t>(CommandQueue::Present)] = m_Queues[static_cast<size_t>(CommandQueue::Graphics)];

            if constexpr (Information::Validation)
            {
                SetDebugName(m_Queues[static_cast<size_t>(CommandQueue::Graphics)].Get(), "Graphics/Present Queue");
                SetDebugName(m_Queues[static_cast<size_t>(CommandQueue::Compute)].Get(), "Compute Queue");
            }
        }

        // Debug message queue callback
        {
            if constexpr (Information::Validation)
            {
                DX_VERIFY(m_Device->QueryInterface(IID_PPV_ARGS(&m_MessageQueue)));
                m_MessageQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE); // Note: We want errors to pass through the message queue into the terminal for ease of debugging
                m_MessageQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
                m_MessageQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            
                OB_ASSERT(m_MessageQueue, "[Dx12Context] Failed to initialize message queue.");
            
                IterateD3D12Messages(m_MessageQueue.Get());
            }
        }

        // Set debug names
        if constexpr (Information::Validation)
        {
            SetDebugName(m_Device.Get(), L"Device");
        }
    }

    Dx12Context::~Dx12Context()
    {
        if constexpr (Information::Validation)
        {
            DxPtr<ID3D12DebugDevice> debugDevice = nullptr;
            DX_VERIFY(m_Device->QueryInterface(IID_PPV_ARGS(&debugDevice)));

            //debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12Context::Warn(const std::string& message) const
    {
#if !defined(OB_CONFIG_DIST)
        if (static_cast<bool>(!s_MessageCallback)) [[unlikely]]
            return;

        s_MessageCallback(DeviceMessageType::Warn, message);
#else
        (void)message;
#endif
    }

    void Dx12Context::Error(const std::string& message) const
    {
#if !defined(OB_CONFIG_DIST)
        if (static_cast<bool>(!s_MessageCallback)) [[unlikely]]
            return;

        s_MessageCallback(DeviceMessageType::Error, message);
#else
        (void)message;
#endif
    }

    void Dx12Context::OutputMessages() const
    {
        IterateD3D12Messages(m_MessageQueue.Get());
    }

    void Dx12Context::Destroy(DeviceDestroyFn fn) const
    {
        m_DestroyCallback(fn);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Internal methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12Context::SetDebugName(ID3D12Object* object, const std::string& name) const
    {
        int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, name.c_str(), static_cast<int>(name.size()), nullptr, 0);

        OB_ASSERT((length != 0), "[Dx12Context] Failed to convert std::string to std::wstring. Error: {0}", GetLastError());

        std::wstring output(length, L'\0');
        (void)MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, name.c_str(), static_cast<int>(name.size()), output.data(), length);
    
        SetDebugName(object, output);
    }

    void Dx12Context::SetDebugName(ID3D12Object* object, const std::wstring& name) const
    {
        object->SetName(name.c_str());
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Internal getters
    ////////////////////////////////////////////////////////////////////////////////////
    bool Dx12Context::AllowsTearing() const
    {
        BOOL allowTearing;
        DX_VERIFY(m_Factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing)));
        return (allowTearing == TRUE);
    }

}