#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

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

    class VulkanDevice;
    class VulkanImGuiRenderer;

#if defined(NG_API_VULKAN)
    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanImGuiRenderer
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanImGuiRenderer
    {
    public:
        // Constructor & Destructor
        VulkanImGuiRenderer(const Device& device, const Swapchain& swapchain, const Renderpass& renderpass);
        ~VulkanImGuiRenderer();

        // Methods
        void Begin() const;
        void End(const CommandList& commandList) const;

    private:
        const VulkanDevice& m_Device;

        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    };
#endif

}