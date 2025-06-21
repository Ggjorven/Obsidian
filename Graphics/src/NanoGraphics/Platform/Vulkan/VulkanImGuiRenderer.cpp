#include "ngpch.h"
#include "VulkanImGuiRenderer.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Window.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/Swapchain.hpp"
#include "NanoGraphics/Renderer/Renderpass.hpp"
#include "NanoGraphics/Renderer/CommandList.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanResources.hpp"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanImGuiRenderer::VulkanImGuiRenderer(const Device& device, const Swapchain& swapchain, const Renderpass& renderpass)
        : m_Device(*api_cast<const VulkanDevice*>(&device))
    {
        const VulkanSwapchain& vulkanSwapchain = *api_cast<const VulkanSwapchain*>(&swapchain);
        const VulkanRenderpass& vulkanRenderpass = *api_cast<const VulkanRenderpass*>(&renderpass);

        // DescriptorPool
        {
            constexpr auto poolSizes = std::to_array<VkDescriptorPoolSize>(
            {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
            });

            VkDescriptorPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = 100 * Information::FramesInFlight;
            poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

            VK_VERIFY(vkCreateDescriptorPool(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &poolInfo, VulkanAllocator::GetCallbacks(), &m_DescriptorPool));
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
            GLFWwindow* window = static_cast<GLFWwindow*>(vulkanSwapchain.GetSpecification().WindowTarget->GetNativeWindow());
            ImGui_ImplGlfw_InitForVulkan(window, true);
        }

        // Imgui renderer
        {
            ImGui_ImplVulkan_InitInfo initInfo = {};
            initInfo.ApiVersion = VK_MAKE_API_VERSION(0, std::get<0>(VulkanContext::Version), std::get<1>(VulkanContext::Version), 0);
            initInfo.Instance = m_Device.GetContext().GetVkInstance();
            initInfo.PhysicalDevice = m_Device.GetContext().GetVulkanPhysicalDevice().GetVkPhysicalDevice();
            initInfo.Device = m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice();
            initInfo.QueueFamily = m_Device.GetContext().GetVulkanPhysicalDevice().GetQueueFamilyIndices().QueueFamily;
            initInfo.Queue = m_Device.GetContext().GetVulkanLogicalDevice().GetVkQueue(CommandQueue::Graphics);
            initInfo.PipelineCache = m_Device.GetAllocator().GetPipelineCache();
            initInfo.DescriptorPool = m_DescriptorPool;
            initInfo.RenderPass = vulkanRenderpass.GetVkRenderPass();
            initInfo.Allocator = VulkanAllocator::GetCallbacks();
            initInfo.MinImageCount = SwapchainSupportDetails::Query(vulkanSwapchain.GetVkSurface(), m_Device.GetContext().GetVulkanPhysicalDevice().GetVkPhysicalDevice()).Capabilities.minImageCount;
            initInfo.ImageCount = static_cast<uint32_t>(vulkanSwapchain.GetImageCount());
            initInfo.CheckVkResultFn = nullptr; // Note: Optional, a callback function for Vulkan errors
            initInfo.MSAASamples = static_cast<VkSampleCountFlagBits>(SampleCountToVkSampleCountFlags(vulkanRenderpass.GetFramebuffer(0).GetSpecification().ColourAttachment.ImagePtr->GetSpecification().SampleCount)); // In case of MSAA
            initInfo.Subpass = 0;

            ImGui_ImplVulkan_Init(&initInfo);

            // Create fonts
            ImGui::GetIO().Fonts->AddFontDefault();
        }
    }

    VulkanImGuiRenderer::~VulkanImGuiRenderer()
    {
        VkDevice vkDevice = m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice();
        m_Device.GetContext().Destroy([vkDevice, descriptorPool = m_DescriptorPool]()
        {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();

            vkDestroyDescriptorPool(vkDevice, descriptorPool, VulkanAllocator::GetCallbacks());
        });
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanImGuiRenderer::Begin() const
    {
        NG_PROFILE("VulkanImGuiLayer::Begin()");
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void VulkanImGuiRenderer::End(const CommandList& commandList) const
    {
        NG_PROFILE("VulkanImGuiLayer::End()");

        ImGuiIO& io = ImGui::GetIO();
        //io.DisplaySize = ImVec2((float)Application::Get().GetWindow().GetWidth(), (float)Application::Get().GetWindow().GetHeight());

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), api_cast<const VulkanCommandList*>(&commandList)->GetVkCommandBuffer());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backupContext = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backupContext);
        }
    }

}