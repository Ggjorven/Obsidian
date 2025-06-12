#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/BufferSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{
    class Device;
}

namespace Nano::Graphics::Internal
{

    class VulkanDevice;

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanFramebuffer
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanBuffer
    {
    public:
        // Constructors & Destructor
        VulkanBuffer(const Device& device, const BufferSpecification& specs);
        ~VulkanBuffer();

        // Getters
        inline const BufferSpecification& GetSpecification() const { return m_Specification; }

        // Internal getters
        inline VkBuffer GetVkBuffer() const { return m_Buffer; }
        inline VmaAllocation GetVmaAllocation() const { return m_Allocation; }

        inline const VulkanDevice& GetVulkanDevice() const { return m_Device; }

    private:
        const VulkanDevice& m_Device;
        BufferSpecification m_Specification;

        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VmaAllocation m_Allocation = VK_NULL_HANDLE;
    };

}