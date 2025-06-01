#pragma once

#include "NanoGraphics/Core/Logging.hpp"

#include <Nano/Nano.hpp>

#include <cstdint>
#include <utility>
#include <array>
#include <tuple>
#include <span>

#if defined(NANO_COMPILER_GCC)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"

    #include <vulkan/vulkan.h>
    #include <vma/vk_mem_alloc.h>

    #pragma GCC diagnostic pop
#elif !defined(NANO_PLATFORM_APPLE)
    #pragma warning(push, 0)

    #include <vulkan/vulkan.h>
    #include <vma/vk_mem_alloc.h>

    #pragma warning(pop)
#else
    #include <vulkan/vulkan.h>
    #include <vma/vk_mem_alloc.h>
#endif

////////////////////////////////////////////////////////////////////////////////////
// Helper
////////////////////////////////////////////////////////////////////////////////////
template<>
struct Nano::Enum::Range<VkResult>
{
public:
    inline static constexpr int32_t Min = -11;  // VK_ERROR_FORMAT_NOT_SUPPORTED
    inline static constexpr int32_t Max = 5;    // VK_INCOMPLETE
};

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Helper macro
    ////////////////////////////////////////////////////////////////////////////////////
    #define VK_VERIFY_IMPL2(expr, str, num)                                                                             \
            VkResult result##num = expr;                                                                                \
            if (result##num != VK_SUCCESS)                                                                              \
            NG_LOG_ERROR("Expression '{0}' failed with error code: {1}", str, ::Nano::Enum::Name(result##num))
    #define VK_VERIFY_IMPL(expr, str, num) VK_VERIFY_IMPL2(expr, str, num)

    #if !defined(NG_CONFIG_DIST)
        #define VK_VERIFY(expr) VK_VERIFY_IMPL((expr), #expr, __COUNTER__)
    #else
        #define VK_VERIFY(expr) expr
    #endif

    ////////////////////////////////////////////////////////////////////////////////////
	// VulkanAllocator
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanAllocator
    {
    public:
        // Init & Destroy
        static void Init();
        static void Destroy();

        // Pipeline Cache
        static VkPipelineCache CreatePipelineCache(std::span<const uint8_t> data);
		inline static VkPipelineCache GetPipelineCache() { return s_PipelineCache; }

        // Buffer
        static VmaAllocation AllocateBuffer(VmaMemoryUsage memoryUsage, VkBuffer& dstBuffer, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags requiredFlags = 0);
        static void CopyBuffer(VkCommandBuffer cmdBuf, VkBuffer& srcBuffer, VkBuffer& dstBuffer, size_t size, size_t offset = 0);
        static void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);

        // Image
        static VmaAllocation AllocateImage(VmaMemoryUsage memUsage, VkImage& image, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags requiredFlags = 0);
		static void CopyBufferToImage(VkCommandBuffer cmdBuf, VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height); // Note: The image will be in VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL after the copy
        static VkImageView CreateImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
        static VkSampler CreateSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressmode, VkSamplerMipmapMode mipmapMode, uint32_t mipLevels);
        static void DestroyImage(VkImage image, VmaAllocation allocation);

        // Utils
        static void MapMemory(VmaAllocation& allocation, void*& mapData);
        static void UnMapMemory(VmaAllocation& allocation);
        static void SetData(VmaAllocation& allocation, void* data, size_t size);
        static void SetMappedData(void* mappedData, void* data, size_t size);

    private:
		inline static VmaAllocator s_Allocator = VK_NULL_HANDLE;
		inline static VkPipelineCache s_PipelineCache = VK_NULL_HANDLE;
    };

}