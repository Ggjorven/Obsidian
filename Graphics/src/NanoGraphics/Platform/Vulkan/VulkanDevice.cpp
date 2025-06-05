#include "ngpch.h"
#include "VulkanDevice.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanDevice::VulkanDevice(const DeviceSpecification& specs)
        : m_Context(specs.NativeWindow, specs.MessageCallback, specs.Extensions), m_Allocator(m_Context.GetVkInstance(), m_Context.GetVulkanPhysicalDevice().GetVkPhysicalDevice(), m_Context.GetVulkanLogicalDevice().GetVkDevice())
    {
    }

    VulkanDevice::~VulkanDevice()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanDevice::Wait() const
    {
        m_Context.GetVulkanLogicalDevice().Wait();
    }

}