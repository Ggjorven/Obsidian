#pragma once

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Information.hpp"

#include <Nano/Nano.hpp>

#include <cstdint>
#include <utility>
#include <array>
#include <tuple>
#include <span>

#if defined(NG_COMPILER_GCC)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"

    #include <vulkan/vulkan.h>
    #include <vma/vk_mem_alloc.h>

    #pragma GCC diagnostic pop
#elif !defined(NG_PLATFORM_APPLE)
    #pragma warning(push, 0)

    #include <vulkan/vulkan.h>
    #include <vma/vk_mem_alloc.h>

    #pragma warning(pop)
#else
    #include <vulkan/vulkan.h>
    #include <vma/vk_mem_alloc.h>
#endif

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Helper macro
    ////////////////////////////////////////////////////////////////////////////////////
    #define VK_VERIFY_IMPL2(expr, str, num)                                                                             \
            VkResult result##num = expr;                                                                                \
            if (result##num != VK_SUCCESS)                                                                              \
            NG_LOG_ERROR("Expression '{0}' failed with error code: {1}", str, ::Nano::Graphics::Internal::VkResultToString(result##num))
    #define VK_VERIFY_IMPL(expr, str, num) VK_VERIFY_IMPL2(expr, str, num)

    #if !defined(NG_CONFIG_DIST)
        #define VK_VERIFY(expr) VK_VERIFY_IMPL((expr), #expr, __COUNTER__)
    #else
        #define VK_VERIFY(expr) expr
    #endif

    ////////////////////////////////////////////////////////////////////////////////////
	// VulkanAllocator
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanAllocator : public Traits::NoCopy // TODO: Update methods
    {
    public:
        // Constructor & Destructor
        VulkanAllocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice logicalDevice);
        ~VulkanAllocator();

        // Pipeline Cache
        VkPipelineCache CreatePipelineCache(VkDevice logicalDevice, std::span<const uint8_t> data);
        VkPipelineCache GetPipelineCache() const;

        // Buffer
        VmaAllocation AllocateBuffer(VmaMemoryUsage memoryUsage, VkBuffer& dstBuffer, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags requiredFlags = 0) const;
        void CopyBuffer(VkCommandBuffer cmdBuf, VkBuffer& srcBuffer, VkBuffer& dstBuffer, size_t size, size_t offset = 0) const;
        void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation) const;

        // Image
        VmaAllocation AllocateImage(VmaMemoryUsage memUsage, VkImage& image, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags requiredFlags = 0) const;
		void CopyBufferToImage(VkCommandBuffer cmdBuf, VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height) const; // Note: The image will be in VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL after the copy
        VkImageView CreateImageView(VkDevice logicalDevice, VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) const;
        VkSampler CreateSampler(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressmode, VkSamplerMipmapMode mipmapMode, uint32_t mipLevels) const;
        void DestroyImage(VkImage image, VmaAllocation allocation) const;

        // Utils
        void MapMemory(VmaAllocation& allocation, void*& mapData) const;
        void UnMapMemory(VmaAllocation& allocation) const;
        void SetData(VmaAllocation& allocation, void* data, size_t size) const;
        void SetMappedData(void* mappedData, void* data, size_t size) const;

        // Getters
        inline const VkAllocationCallbacks* GetCallbacks() const { return &m_Callbacks; }

    private:
		VmaAllocator m_Allocator = VK_NULL_HANDLE;
		VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;

        VkAllocationCallbacks m_Callbacks = {};
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ToString methods
    ////////////////////////////////////////////////////////////////////////////////////
    inline constexpr std::string_view VkResultToString(VkResult result)
    {
        switch (result)
        {
        case VK_SUCCESS:                                            return "VK_SUCCESS";
        case VK_NOT_READY:                                          return "VK_NOT_READY";
        case VK_TIMEOUT:                                            return "VK_TIMEOUT";
        case VK_EVENT_SET:                                          return "VK_EVENT_SET";
        case VK_EVENT_RESET:                                        return "VK_EVENT_RESET";
        case VK_INCOMPLETE:                                         return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY:                           return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:                         return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:                        return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST:                                  return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED:                            return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:                            return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:                        return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT:                          return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:                          return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS:                             return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:                         return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL:                              return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_UNKNOWN:                                      return "VK_ERROR_UNKNOWN";
        case VK_ERROR_OUT_OF_POOL_MEMORY:                           return "VK_ERROR_OUT_OF_POOL_MEMORY";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:                      return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
        case VK_ERROR_FRAGMENTATION:                                return "VK_ERROR_FRAGMENTATION";
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:               return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
        case VK_ERROR_SURFACE_LOST_KHR:                             return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:                     return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR:                                     return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR:                              return "VK_ERROR_OUT_OF_DATE_KHR";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:                     return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case VK_ERROR_VALIDATION_FAILED_EXT:                        return "VK_ERROR_VALIDATION_FAILED_EXT";
        case VK_ERROR_INVALID_SHADER_NV:                            return "VK_ERROR_INVALID_SHADER_NV";
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
        case VK_ERROR_NOT_PERMITTED_EXT:                            return "VK_ERROR_NOT_PERMITTED_EXT";
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:          return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
        case VK_THREAD_IDLE_KHR:                                    return "VK_THREAD_IDLE_KHR";
        case VK_THREAD_DONE_KHR:                                    return "VK_THREAD_DONE_KHR";
        case VK_OPERATION_DEFERRED_KHR:                             return "VK_OPERATION_DEFERRED_KHR";
        case VK_OPERATION_NOT_DEFERRED_KHR:                         return "VK_OPERATION_NOT_DEFERRED_KHR";
        case VK_PIPELINE_COMPILE_REQUIRED_EXT:                      return "VK_PIPELINE_COMPILE_REQUIRED_EXT";

        default:
            break;
        }

        NG_UNREACHABLE();
        return {};
    }


}