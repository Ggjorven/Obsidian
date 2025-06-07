#include "ngpch.h"
#include "VulkanImage.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Core/Window.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/Swapchain.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"

namespace Nano::Graphics::Internal
{

    static_assert(std::is_same_v<Device::Type, VulkanDevice>, "Current Device::Type is not VulkanDevice and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<Swapchain::Type, VulkanSwapchain>, "Swapchain Image::Type is not VulkanSwapchain and Vulkan source code is being compiled.");

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanSwapchain::VulkanSwapchain(const Device& device, const SwapchainSpecification& specs)
        : m_Device(*reinterpret_cast<const VulkanDevice*>(&device)), m_Specification(specs)
    {
        Resize(m_Specification.WindowTarget->GetSize().x, m_Specification.WindowTarget->GetSize().y, m_Specification.VSync, m_Specification.RequestedFormat, m_Specification.RequestedColourSpace);
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanSwapchain::Resize(uint32_t width, uint32_t height)
    {
        Resize(width, height, m_Specification.VSync, m_Specification.RequestedFormat, m_Specification.RequestedColourSpace);
    }

    void VulkanSwapchain::Resize(uint32_t width, uint32_t height, bool vsync, Format colourFormat, ColourSpace colourSpace)
    {
        if (width == 0 || height == 0)
            return;

        if ((colourFormat != m_Specification.RequestedFormat) || (colourSpace != m_Specification.RequestedColourSpace)) [[unlikely]]
        {
            // TODO: ...
        }
    }

}