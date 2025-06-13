#include "ngpch.h"
#include "VulkanBuffer.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"

namespace Nano::Graphics::Internal
{

    static_assert(std::is_same_v<Device::Type, VulkanDevice>, "Current Device::Type is not VulkanDevice and Vulkan source code is being compiled.");

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanBuffer::VulkanBuffer(const Device& device, const BufferSpecification& specs)
        : m_Device(*reinterpret_cast<const VulkanDevice*>(&device)), m_Specification(specs)
    {
        // Validation checks
        if constexpr (VulkanContext::Validation)
        {
            if (m_Specification.KeepResourceState && m_Specification.State == ResourceState::Unknown)
            {
                m_Device.GetContext().Error("[VulkanBuffer] KeepResourceState = true, but ResourceState is set to Unknowm which is not compatible. Disable KeepResourceState.");
            }
        }

        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

        // MemoryUsage
        {
            if (static_cast<bool>((m_Specification.CpuAccess & CpuAccessMode::Read)) && static_cast<bool>((m_Specification.CpuAccess & CpuAccessMode::Write))
                || static_cast<bool>(m_Specification.CpuAccess & CpuAccessMode::Write))
                memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            else if (static_cast<bool>(m_Specification.CpuAccess & CpuAccessMode::Read))
                memoryUsage = VMA_MEMORY_USAGE_GPU_TO_CPU;
        }

        VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        
        // BufferUsage
        {
            if (m_Specification.IsVertexBuffer)
                bufferUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            if (m_Specification.IsIndexBuffer)
                bufferUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            if (m_Specification.IsUniformBuffer)
                bufferUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

            if (m_Specification.IsUnorderedAccessed)
                bufferUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }

        m_Allocation = m_Device.GetAllocator().AllocateBuffer(memoryUsage, m_Buffer, m_Specification.Size, bufferUsage, 0);

        if constexpr (VulkanContext::Validation)
        {
            if (!m_Specification.DebugName.empty())
                m_Device.GetContext().SetDebugName(m_Buffer, VK_OBJECT_TYPE_BUFFER, std::string(m_Specification.DebugName));
        }
    }

    VulkanBuffer::~VulkanBuffer()
    {
    }

}