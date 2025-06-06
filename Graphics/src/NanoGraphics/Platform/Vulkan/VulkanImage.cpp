#include "ngpch.h"
#include "VulkanImage.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/Image.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanResource.hpp"

namespace Nano::Graphics::Internal
{

    static_assert(std::is_same_v<Device::Type, VulkanDevice>, "Current Device::Type is not VulkanDevice and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<Image::Type, VulkanImage>, "Current Image::Type is not VulkanImage and Vulkan source code is being compiled.");

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanImageSubresourceView::VulkanImageSubresourceView(const Image& image, const ImageSubresourceSpecification& specs)
        : m_Image(*reinterpret_cast<const VulkanImage*>(&image)), m_Specification(specs)
    {
    }

    VulkanImageSubresourceView::~VulkanImageSubresourceView()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanImage::VulkanImage(const Device& device, const ImageSpecification& specs)
        : m_Device(*reinterpret_cast<const VulkanDevice*>(&device)), m_Specification(specs)
    {
        // Validation checks
        if (m_Specification.KeepResourceState && m_Specification.State == ResourceState::Unknown)
        {
            m_Device.GetContext().Error("[VulkanImage] KeepResourceState = true, but ResourceState is set to Unknowm which is not compatible. Disable KeepResourceState.");
        }

    }

    VulkanImage::~VulkanImage()
    {
    }

}