#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"

#include <Nano/Nano.hpp>

#include <imgui.h>

namespace Nano::Graphics
{
    class Device;
    class Swapchain;
    class Renderpass;
    class CommandList;
}

namespace Nano::Graphics::Internal
{

    class Dx12Device;
    class Dx12ImGuiRenderer;

#if defined(NG_API_DX12)
    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12ImGuiRenderer
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12ImGuiRenderer
    {
    public:
        // Constructor & Destructor
        Dx12ImGuiRenderer(const Device& device, const Swapchain& swapchain, const Renderpass& renderpass);
        ~Dx12ImGuiRenderer();

        // Methods
        void Begin() const;
        void End(const CommandList& commandList) const;

    private:
        const Dx12Device& m_Device;

        DxPtr<ID3D12DescriptorHeap> m_DescriptorHeap = nullptr;;
    };
#endif

}