#include "ngpch.h"
#include "VulkanBuffer.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"

#include <unordered_map>

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanInputLayout::VulkanInputLayout(const Device& device, std::span<const VertexAttributeSpecification> attributes)
        : m_Attributes(attributes.begin(), attributes.end())
    {
        (void)device;

        CalculateOffsetsAndStride();

        uint32_t totalAttributeArraySize = 0;

        // Create a binging map
        std::unordered_map<uint32_t, VkVertexInputBindingDescription> bindingMap;
        bindingMap.reserve(attributes.size());

        for (const auto& spec : m_Attributes)
        {
            NG_ASSERT((spec.ArraySize > 0), "[VkInputLayout] A vertex attribute specification must be more than 0.");
            totalAttributeArraySize += spec.ArraySize;

            if (!bindingMap.contains(spec.BufferIndex))
            {
                VkVertexInputBindingDescription bindingDescription = {};
                bindingDescription.binding = spec.BufferIndex;
                bindingDescription.stride = m_Stride;
                bindingDescription.inputRate = ((spec.IsInstanced) ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX);

                bindingMap[spec.BufferIndex] = bindingDescription;
            }
            else 
            {
                NG_ASSERT((bindingMap[spec.BufferIndex].inputRate == (spec.IsInstanced ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX)), "[VkInputLayout] All vertex attributes must have the same input rate.");
            }
        }

        // Add all bindings to vector
        m_BindingDescriptions.reserve(bindingMap.size());
        for (const auto& [_, binding] : bindingMap)
            m_BindingDescriptions.push_back(binding);

        // build attribute descriptions
        m_AttributeDescriptions.resize(static_cast<size_t>(totalAttributeArraySize));

        uint32_t attributeLocation = 0;
        for (const VertexAttributeSpecification& spec : m_Attributes)
        {
            uint32_t elementSize = FormatToFormatInfo(spec.VertexFormat).BytesPerBlock;
            uint32_t bufferOffset = 0;

            for (uint32_t slot = 0; slot < spec.ArraySize; slot++)
            {
                auto& outAttrib = m_AttributeDescriptions[attributeLocation];

                outAttrib.location = attributeLocation;
                outAttrib.binding = spec.BufferIndex;
                outAttrib.format = FormatToVkFormat(spec.VertexFormat);
                outAttrib.offset = bufferOffset + spec.Offset;
                bufferOffset += elementSize;

                ++attributeLocation;
            }
        }
    }

    VulkanInputLayout::~VulkanInputLayout()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanInputLayout::CalculateOffsetsAndStride()
    {
        uint32_t offset = 0;
        uint32_t maxOffsetEnd = 0;

        for (auto& attribute : m_Attributes)
        {
            if (attribute.Size == VertexAttributeSpecification::AutoSize)
                attribute.Size = FormatToFormatInfo(attribute.VertexFormat).BytesPerBlock;
            if (attribute.Offset == VertexAttributeSpecification::AutoOffset)
                attribute.Offset = offset;

            offset = attribute.Offset + attribute.Size;
            maxOffsetEnd = std::max(maxOffsetEnd, offset);
        }

        m_Stride = maxOffsetEnd;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanBuffer::VulkanBuffer(const Device& device, const BufferSpecification& specs)
        : m_Specification(specs)
    {
        NG_ASSERT((specs.Size != 0), "[VkBuffer] Size must not equal 0.");

        const VulkanDevice& vulkanDevice = *api_cast<const VulkanDevice*>(&device);

        // Validation checks
        if constexpr (VulkanContext::Validation)
        {
            if (m_Specification.IsIndexBuffer)
            {
                NG_ASSERT(((m_Specification.BufferFormat == Format::R16UInt) || (m_Specification.BufferFormat == Format::R32UInt)), "[VkBuffer] An indexbuffer must have format R16UInt or R32UInt.");
            }
        }

        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;

        // MemoryUsage
        {
            if ((static_cast<bool>((m_Specification.CpuAccess & CpuAccessMode::Read)) && static_cast<bool>((m_Specification.CpuAccess & CpuAccessMode::Write)))
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
            if (m_Specification.IsUniformBuffer && m_Specification.IsTexel)
                bufferUsage |= VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
            else if (m_Specification.IsUniformBuffer)
                bufferUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

            if (m_Specification.IsUnorderedAccessed && m_Specification.IsTexel)
                bufferUsage |= VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
            else if (m_Specification.IsUnorderedAccessed)
                bufferUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }

        size_t size = 0;
        // Size helper
        {
            if (m_Specification.IsDynamic)
            {
                NG_ASSERT((m_Specification.DynamicElements != 0), "[VkBuffer] Dynamic element count must not equal zero when dynamic is enabled.");

                VkPhysicalDeviceProperties2 properties = {};
                properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

                vkGetPhysicalDeviceProperties2(vulkanDevice.GetContext().GetVulkanPhysicalDevice().GetVkPhysicalDevice(), &properties);

                uint64_t alignment = properties.properties.limits.minUniformBufferOffsetAlignment;
                uint64_t atomSize = properties.properties.limits.nonCoherentAtomSize;

                alignment = std::max(alignment, atomSize);

                NG_ASSERT(((alignment & (alignment - 1)) == 0), "[VkBuffer] Internal error: Alignment must be a power of 2.");

                size = (size + alignment - 1) & ~(alignment - 1);
                m_Specification.Size = size;

                size *= m_Specification.DynamicElements;

                if constexpr (VulkanContext::Validation)
                {
                    if (!static_cast<bool>(m_Specification.CpuAccess & CpuAccessMode::Write))
                    {
                        vulkanDevice.GetContext().Warn("[VkBuffer] Creating a Dynamic buffer with out CpuAccessMode::Write flag. This must be added.");
                        m_Specification.CpuAccess |= CpuAccessMode::Write;
                    }
                }
            }
            else if (m_Specification.Size < 65536) // Vulkan allows for <= 64kb buffer updates to be done inline via vkCmdUpdateBuffer, but must be a multiple of 4
                size = (m_Specification.Size + 3) & ~3ull;
            else
                size = m_Specification.Size;
        }

        m_Allocation = vulkanDevice.GetAllocator().AllocateBuffer(memoryUsage, m_Buffer, size, bufferUsage, 0);

        if constexpr (VulkanContext::Validation)
        {
            if (!m_Specification.DebugName.empty())
            {
                vulkanDevice.GetContext().SetDebugName(m_Buffer, VK_OBJECT_TYPE_BUFFER, std::string(m_Specification.DebugName));
                vulkanDevice.GetContext().SetDebugName(vulkanDevice.GetAllocator().GetUnderlyingMemory(m_Allocation), VK_OBJECT_TYPE_DEVICE_MEMORY, std::format("Memory for: {0}", m_Specification.DebugName));
            }
        }
    }

    VulkanBuffer::~VulkanBuffer()
    {
    }

}