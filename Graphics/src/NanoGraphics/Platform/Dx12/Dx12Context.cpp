#include "ngpch.h"
#include "Dx12Context.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include <type_traits>

namespace Nano::Graphics::Internal
{

    namespace
    {

        ////////////////////////////////////////////////////////////////////////////////////
        // Static callback/functions
        ////////////////////////////////////////////////////////////////////////////////////
        static Nano::Graphics::DeviceMessageCallback s_MessageCallback = {};

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

                std::vector<uint8_t> messageData(messageLength);
                auto* message = reinterpret_cast<D3D12_MESSAGE*>(messageData.data());

                DeviceMessageType messageType;
                switch (message->Severity) 
                {
                case D3D12_MESSAGE_SEVERITY_CORRUPTION: messageType = DeviceMessageType::Error;     break;
                case D3D12_MESSAGE_SEVERITY_ERROR:      messageType = DeviceMessageType::Error;     break;
                case D3D12_MESSAGE_SEVERITY_WARNING:    messageType = DeviceMessageType::Warn;   break;
                case D3D12_MESSAGE_SEVERITY_INFO:       messageType = DeviceMessageType::Info;      break;
                case D3D12_MESSAGE_SEVERITY_MESSAGE:    messageType = DeviceMessageType::Trace;   break;

                default:
                    NG_UNREACHABLE();
                    break;
                }

                s_MessageCallback(messageType, message->pDescription);
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
        if constexpr (Information::Validation)
        {
            DX_VERIFY(D3D12GetDebugInterface(IID_PPV_ARGS(&m_DebugController)));
            m_DebugController->EnableDebugLayer();
        }
    }

    Dx12Context::~Dx12Context()
    {
        if constexpr (Information::Validation)
        {
            Destroy([messageQueue = m_MessageQueue, debugController = m_DebugController]()
            {
                messageQueue->Release();
                debugController->Release();
            });
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12Context::Warn(const std::string& message)
    {
#if !defined(NG_CONFIG_DIST)
        if (static_cast<bool>(!s_MessageCallback)) [[unlikely]]
            return;

        s_MessageCallback(DeviceMessageType::Warn, message);
#else
        (void)message;
#endif
    }

    void Dx12Context::Error(const std::string& message)
    {
#if !defined(NG_CONFIG_DIST)
        if (static_cast<bool>(!s_MessageCallback)) [[unlikely]]
            return;

        s_MessageCallback(DeviceMessageType::Error, message);
#else
        (void)message;
#endif
    }

    void Dx12Context::Destroy(DeviceDestroyFn fn)
    {
        m_DestroyCallback(fn);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Internal methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12Context::InitMessageQueue(ID3D12Device* device)
    {
        if constexpr (Information::Validation)
        {
            DX_VERIFY(device->QueryInterface(IID_PPV_ARGS(&m_MessageQueue)));
        
            m_MessageQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
            m_MessageQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
            m_MessageQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, FALSE);

            IterateD3D12Messages(m_MessageQueue);
        }
    }

    void Dx12Context::SetDebugName(ID3D12Object* object, const std::wstring& name)
    {
        object->SetName(name.c_str());
    }

}