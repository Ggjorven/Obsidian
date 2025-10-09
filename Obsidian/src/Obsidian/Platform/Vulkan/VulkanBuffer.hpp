#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/API.hpp"
#include "Obsidian/Renderer/BufferSpec.hpp"

#include "Obsidian/Platform/Vulkan/Vulkan.hpp"

#include <Nano/Nano.hpp>

#include <span>

namespace Obsidian
{
    class Device;
}

namespace Obsidian::Internal
{

    class VulkanDevice;
    class VulkanInputLayout;
    class VulkanBuffer;

#if defined(OB_API_VULKAN)
    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanInputLayout
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanInputLayout
    {
    public:
        // Constructor & Destructor
        VulkanInputLayout(const Device& device, std::span<const VertexAttributeSpecification> attributes);
        ~VulkanInputLayout();

        // Internal getters
        inline uint32_t GetStride() const { return m_Stride; }
        inline const std::vector<VkVertexInputBindingDescription>& GetBindingDescriptions() const { return m_BindingDescriptions; }
        inline const std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions() const { return m_AttributeDescriptions; }

    private:
        // Private methods
        void CalculateOffsetsAndStride();

    private:
        std::vector<VertexAttributeSpecification> m_Attributes;

        uint32_t m_Stride = 0;

        std::vector<VkVertexInputBindingDescription> m_BindingDescriptions = { };
        std::vector<VkVertexInputAttributeDescription> m_AttributeDescriptions = { };
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanBuffer
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanBuffer
    {
    public:
        // Constructor & Destructor
        VulkanBuffer(const Device& device, const BufferSpecification& specs);
        ~VulkanBuffer();

        // Getters
        inline const BufferSpecification& GetSpecification() const { return m_Specification; }
        
        inline size_t GetAlignment() const { return m_Alignment; }

        // Internal getters
        inline VkBuffer GetVkBuffer() const { return m_Buffer; }
        inline VmaAllocation GetVmaAllocation() const { return m_Allocation; }

    private:
        BufferSpecification m_Specification;
        size_t m_Alignment = 0;

        VkBuffer m_Buffer = VK_NULL_HANDLE;
        VmaAllocation m_Allocation = VK_NULL_HANDLE;

        // Note: Maybe in the future add BufferViews like ImageViews
    };
#endif

}