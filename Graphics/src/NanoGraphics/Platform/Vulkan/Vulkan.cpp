#include "ngpch.h"
#include "Vulkan.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"

#if defined(NG_COMPILER_GCC)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-variable"
	#pragma GCC diagnostic ignored "-Wunused-parameter"
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

	#define VMA_IMPLEMENTATION
	#include <vma/vk_mem_alloc.h>

	#pragma GCC diagnostic pop
#elif !defined(NG_PLATFORM_APPLE)
	#pragma warning(push, 0)

	#define VMA_IMPLEMENTATION
	#include <vma/vk_mem_alloc.h>

	#pragma warning(pop)
#else
	#define VMA_IMPLEMENTATION
	#include <vma/vk_mem_alloc.h>
#endif

namespace
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Callbacks
    ////////////////////////////////////////////////////////////////////////////////////
    static void* VKAPI_PTR VmaAllocFn(void*, size_t size, size_t, VkSystemAllocationScope)
    {
        return std::malloc(size);
    }

    static void VKAPI_PTR VmaFreeFn(void*, void* pMemory)
    {
        std::free(pMemory);
    }

    static void* VKAPI_PTR VmaReallocFn(void*, void* pOriginal, size_t size, size_t, VkSystemAllocationScope)
    {
        return std::realloc(pOriginal, size);
    }

}

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanAllocator::VulkanAllocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice logicalDevice)
    {
        VkAllocationCallbacks callbacks = {};
        callbacks.pUserData = nullptr;
        callbacks.pfnAllocation = &VmaAllocFn;
        callbacks.pfnFree = &VmaFreeFn;
        callbacks.pfnReallocation = &VmaReallocFn;
        callbacks.pfnInternalAllocation = nullptr;
        callbacks.pfnInternalFree = nullptr;

        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.instance = instance;
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = logicalDevice;
        allocatorInfo.pAllocationCallbacks = &callbacks;

        VK_VERIFY(vmaCreateAllocator(&allocatorInfo, &m_Allocator));
    }

    VulkanAllocator::~VulkanAllocator()
    {
        vmaDestroyAllocator(m_Allocator);
        m_Allocator = VK_NULL_HANDLE;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Pipeline Cache
    ////////////////////////////////////////////////////////////////////////////////////
    VkPipelineCache VulkanAllocator::CreatePipelineCache(VkDevice logicalDevice, std::span<const uint8_t> data)
    {
        VkPipelineCacheCreateInfo cacheCreateInfo = {};
        cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        cacheCreateInfo.initialDataSize = data.size();
        cacheCreateInfo.pInitialData = data.data();

        VK_VERIFY(vkCreatePipelineCache(logicalDevice, &cacheCreateInfo, nullptr, &m_PipelineCache));
        return m_PipelineCache;
    }

    VkPipelineCache VulkanAllocator::GetPipelineCache() const
    {
        return m_PipelineCache;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Buffer
    ////////////////////////////////////////////////////////////////////////////////////
    VmaAllocation VulkanAllocator::AllocateBuffer(VmaMemoryUsage memoryUsage, VkBuffer& dstBuffer, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags requiredFlags) const
    {
        NG_PROFILE("VkAllocator::AllocateBuffer()");

        NG_ASSERT(m_Allocator, "[VkAllocator] Allocator not initialized.");
		NG_ASSERT((size > 0), "[VkAllocator] Invalid size passed in for buffer allocation.");

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = static_cast<VkDeviceSize>(size);
        bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Note: On most modern system all queue family indices are the same for graphics & compute, so we can use exclusive sharing mode between these queue's (since they are the same). If you need to use different queue families, then you need to use concurrent sharing mode and specify the queue family indices in the pQueueFamilyIndices field.

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = memoryUsage; 
        allocInfo.requiredFlags = requiredFlags;

        VmaAllocation allocation = VK_NULL_HANDLE;
        VK_VERIFY(vmaCreateBuffer(m_Allocator, &bufferInfo, &allocInfo, &dstBuffer, &allocation, nullptr));

        return allocation;
    }

    void VulkanAllocator::CopyBuffer(VkCommandBuffer cmdBuf, VkBuffer& srcBuffer, VkBuffer& dstBuffer, size_t size, size_t offset) const
    {
        NG_PROFILE("VkAllocator::CopyBuffer()");

		NG_ASSERT((srcBuffer != VK_NULL_HANDLE), "[VkAllocator] Invalid source buffer passed in.");
		NG_ASSERT((dstBuffer != VK_NULL_HANDLE), "[VkAllocator] Invalid destination buffer passed in.");
		NG_ASSERT((size > 0), "[VkAllocator] Invalid size passed in for buffer copy.");
		NG_ASSERT((size > offset), "[VkAllocator] Invalid offset passed in for buffer copy.");

        VkBufferCopy copyRegion = {};
        copyRegion.size = size;
        copyRegion.dstOffset = offset;
        vkCmdCopyBuffer(cmdBuf, srcBuffer, dstBuffer, 1, &copyRegion);
    }

    void VulkanAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation) const
    {
        NG_PROFILE("VkAllocator::DestroyBuffer()");

        NG_ASSERT(m_Allocator, "[VkAllocator] Allocator not initialized.");
		NG_ASSERT((buffer != VK_NULL_HANDLE), "[VkAllocator] Invalid buffer passed in.");
		NG_ASSERT((allocation != VK_NULL_HANDLE), "[VkAllocator] Invalid allocation passed in.");

        vmaDestroyBuffer(m_Allocator, buffer, allocation);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Image
    ////////////////////////////////////////////////////////////////////////////////////
    VmaAllocation VulkanAllocator::AllocateImage(VmaMemoryUsage memUsage, VkImage& image, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags requiredFlags) const
    {
        NG_PROFILE("VkAllocator::AllocateImage()");

		NG_ASSERT(m_Allocator, "[VkAllocator] Allocator not initialized.");
		NG_ASSERT((width > 0) && (height > 0), "[VkAllocator] Invalid width or height passed in for image allocation.");

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // Note/TODO: This should maybe be made a parameter
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Note: On most modern system all queue family indices are the same for graphics & compute, so we can use exclusive sharing mode between these queue's (since they are the same). If you need to use different queue families, then you need to use concurrent sharing mode and specify the queue family indices in the pQueueFamilyIndices field.

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = memUsage;
        allocCreateInfo.requiredFlags = requiredFlags;

        VmaAllocation allocation = VK_NULL_HANDLE;
        VK_VERIFY(vmaCreateImage(m_Allocator, &imageInfo, &allocCreateInfo, &image, &allocation, nullptr));

        return allocation;
    }

    void VulkanAllocator::CopyBufferToImage(VkCommandBuffer cmdBuf, VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height) const
    {
        NG_PROFILE("VkAllocator::CopyBufferToImage()");

        NG_ASSERT((width > 0) && (height > 0), "[VkAllocator] Invalid width or height passed in for copy operation.");

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // TODO: Make parameters
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(cmdBuf, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    VkImageView VulkanAllocator::CreateImageView(VkDevice logicalDevice, VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) const
    {
        NG_PROFILE("VkAllocator::CreateImageView()");

		NG_ASSERT((image != VK_NULL_HANDLE), "[VkAllocator] Invalid image passed in.");

        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.aspectMask = aspectFlags;

        VkImageView imageView = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView));

        return imageView;
    }

    VkSampler VulkanAllocator::CreateSampler(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressmode, VkSamplerMipmapMode mipmapMode, uint32_t mipLevels) const
    {
        NG_PROFILE("VkAllocator::CreateSampler()");

        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = magFilter;
        samplerInfo.minFilter = minFilter;
        samplerInfo.addressModeU = addressmode;
        samplerInfo.addressModeV = addressmode;
        samplerInfo.addressModeW = addressmode;

        VkPhysicalDeviceProperties properties = {};
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);

        samplerInfo.anisotropyEnable = VK_TRUE;                             // Can be disabled: just set VK_FALSE
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // And 1.0f

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = mipmapMode;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevels);
        samplerInfo.mipLodBias = 0.0f; // Optional

        VkSampler sampler = VK_NULL_HANDLE;
        VK_VERIFY(vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &sampler));

        return sampler;
    }

    void VulkanAllocator::DestroyImage(VkImage image, VmaAllocation allocation) const
    {
        NG_PROFILE("VkAllocator::DestroyImage()");

		NG_ASSERT(m_Allocator, "[VkAllocator] Allocator not initialized.");
		NG_ASSERT((image != VK_NULL_HANDLE), "[VkAllocator] Invalid image passed in.");
		NG_ASSERT((allocation != VK_NULL_HANDLE), "[VkAllocator] Invalid allocation passed in.");

        vmaDestroyImage(m_Allocator, image, allocation);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Utils
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanAllocator::MapMemory(VmaAllocation& allocation, void*& mapData) const
    {
        NG_PROFILE("VkAllocator::MapMemory()");

		NG_ASSERT(m_Allocator, "[VkAllocator] Allocator not initialized.");
		NG_ASSERT((allocation != VK_NULL_HANDLE), "[VkAllocator] Invalid allocation passed in.");

        vmaMapMemory(m_Allocator, allocation, &mapData);
    }

    void VulkanAllocator::UnMapMemory(VmaAllocation& allocation) const
    {
        NG_PROFILE("VkAllocator::MapMemory()");

        NG_ASSERT(m_Allocator, "[VkAllocator] Allocator not initialized.");
		NG_ASSERT((allocation != VK_NULL_HANDLE), "[VkAllocator] Invalid allocation passed in.");

        vmaUnmapMemory(m_Allocator, allocation);
    }

    void VulkanAllocator::SetData(VmaAllocation& allocation, void* data, size_t size) const
    {
        NG_PROFILE("VkAllocator::SetData()");

		NG_ASSERT((allocation != VK_NULL_HANDLE), "[VkAllocator] Invalid allocation passed in.");
		NG_ASSERT((data != nullptr), "[VkAllocator] Invalid data passed in.");
		NG_ASSERT((size > 0), "[VkAllocator] Invalid size passed in for data.");

        void* mappedData;

        VulkanAllocator::MapMemory(allocation, mappedData);
        VulkanAllocator::SetMappedData(mappedData, data, size);
        VulkanAllocator::UnMapMemory(allocation);
    }

    void VulkanAllocator::SetMappedData(void* mappedData, void* data, size_t size) const
    {
		NG_PROFILE("VkAllocator::SetMappedData()");

		NG_ASSERT((mappedData != nullptr), "[VkAllocator] Invalid mapped data passed in.");
        NG_ASSERT((data != nullptr), "[VkAllocator] Invalid data passed in.");
        NG_ASSERT((size > 0), "[VkAllocator] Invalid size passed in for data.");

		std::memcpy(mappedData, data, size);
    }

}