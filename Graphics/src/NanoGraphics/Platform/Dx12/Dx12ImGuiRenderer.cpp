#include "ngpch.h"
#include "Dx12ImGuiRenderer.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Window.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/Swapchain.hpp"
#include "NanoGraphics/Renderer/Renderpass.hpp"
#include "NanoGraphics/Renderer/CommandList.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12Resources.hpp"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_dx12.h>

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12ImGuiRenderer::Dx12ImGuiRenderer(const Device& device, const Swapchain& swapchain, const Renderpass& renderpass)
        : m_Device(*api_cast<const Dx12Device*>(&device))
    {
        (void)renderpass;

        const Dx12Swapchain& dxSwapchain = *api_cast<const Dx12Swapchain*>(&swapchain);

        // DescriptorHeap
        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = 100;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            desc.NodeMask = 0;

            DX_VERIFY(m_Device.GetContext().GetD3D12Device()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_DescriptorHeap)));
        }

        // Styling
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

            io.IniFilename = NULL;

            ImGui::StyleColorsDark();

            ImGuiStyle& style = ImGui::GetStyle();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) // When viewports are a thing remove rounding to make every window look identical
            {
                style.WindowRounding = 0.0f;
                style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            }
        }

        // Imgui window
        {
            GLFWwindow* window = static_cast<GLFWwindow*>(dxSwapchain.GetSpecification().WindowTarget->GetNativeWindow());
            ImGui_ImplGlfw_InitForOther(window, true);
        }

        // Imgui renderer
        {
            ImGui_ImplDX12_InitInfo info = {};
            info.Device = m_Device.GetContext().GetD3D12Device().Get();
            info.CommandQueue = m_Device.GetContext().GetD3D12CommandQueue(CommandQueue::Graphics).Get();
            info.NumFramesInFlight = Information::FramesInFlight;
            info.RTVFormat = FormatToFormatMapping(dxSwapchain.GetSpecification().RequestedFormat).RTVFormat;
            info.SrvDescriptorHeap = m_DescriptorHeap.Get();
            info.LegacySingleSrvCpuDescriptor = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            info.LegacySingleSrvGpuDescriptor = m_DescriptorHeap->GetGPUDescriptorHandleForHeapStart();

            ImGui_ImplDX12_Init(&info);

            // Create fonts
            ImGui::GetIO().Fonts->AddFontDefault();
        }
    }

    Dx12ImGuiRenderer::~Dx12ImGuiRenderer()
    {
        m_Device.GetContext().Destroy([]()
        {
            ImGui_ImplDX12_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        });
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12ImGuiRenderer::Begin() const
    {
        NG_PROFILE("Dx12ImGuiLayer::Begin()");
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Dx12ImGuiRenderer::End(const CommandList& commandList) const
    {
        NG_PROFILE("Dx12ImGuiLayer::End()");

        ImGuiIO& io = ImGui::GetIO();
        //io.DisplaySize = ImVec2((float)Application::Get().GetWindow().GetWidth(), (float)Application::Get().GetWindow().GetHeight());

        auto dxCommandList = api_cast<const Dx12CommandList*>(&commandList)->GetID3D12GraphicsCommandList().Get();

        ID3D12DescriptorHeap* heaps[] = { m_DescriptorHeap.Get() };
        dxCommandList->SetDescriptorHeaps(1, heaps);

        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), api_cast<const Dx12CommandList*>(&commandList)->GetID3D12GraphicsCommandList().Get());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backupContext = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backupContext);
        }
    }

}