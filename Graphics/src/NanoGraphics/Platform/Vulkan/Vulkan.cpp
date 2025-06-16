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
    static void* VKAPI_PTR VmaAllocFn(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
    {
        (void)pUserData; (void)alignment; (void)allocationScope;
        return std::malloc(size);
    }

    static void VKAPI_PTR VmaFreeFn(void* pUserData, void* pMemory)
    {
        (void)pUserData;
        std::free(pMemory);
    }

    static void* VKAPI_PTR VmaReallocFn(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
    {
        (void)pUserData; (void)alignment; (void)allocationScope;
        return std::realloc(pOriginal, size);
    }

    static void VKAPI_PTR VmaInternalAllocFn(void* pUserData, size_t size, VkInternalAllocationType type, VkSystemAllocationScope allocationScope)
    {
        (void)pUserData; (void)size; (void)type; (void)allocationScope;
    }

    static void VKAPI_PTR VmaInternalFreeFn(void* pUserData, size_t size, VkInternalAllocationType type, VkSystemAllocationScope allocationScope)
    {
        (void)pUserData; (void)size; (void)type; (void)allocationScope;
    }

}

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanAllocator::VulkanAllocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice logicalDevice)
        : m_Device(logicalDevice)
    {
        s_Callbacks.pUserData = nullptr;
        s_Callbacks.pfnAllocation = &VmaAllocFn;
        s_Callbacks.pfnFree = &VmaFreeFn;
        s_Callbacks.pfnReallocation = &VmaReallocFn;
        s_Callbacks.pfnInternalAllocation = &VmaInternalAllocFn;
        s_Callbacks.pfnInternalFree = &VmaInternalFreeFn;

        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.instance = instance;
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = logicalDevice;
        allocatorInfo.pAllocationCallbacks = &s_Callbacks;

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
    VkPipelineCache VulkanAllocator::CreatePipelineCache(std::span<const uint8_t> data)
    {
        VkPipelineCacheCreateInfo cacheCreateInfo = {};
        cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        cacheCreateInfo.initialDataSize = data.size();
        cacheCreateInfo.pInitialData = data.data();

        VK_VERIFY(vkCreatePipelineCache(m_Device, &cacheCreateInfo, &s_Callbacks, &m_PipelineCache));
        return m_PipelineCache;
    }

    VkPipelineCache VulkanAllocator::GetPipelineCache() const
    {
        return m_PipelineCache;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Buffer
    ////////////////////////////////////////////////////////////////////////////////////
    VmaAllocation VulkanAllocator::AllocateBuffer(VmaMemoryUsage memoryUsage, VkBuffer& buffer, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags requiredFlags) const
    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Change if necessary

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = memoryUsage; // VMA_MEMORY_USAGE_GPU_ONLY, VMA_MEMORY_USAGE_CPU_ONLY, etc.
        allocInfo.requiredFlags = requiredFlags;

        VmaAllocation allocation = VK_NULL_HANDLE;
        VK_VERIFY(vmaCreateBuffer(m_Allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr));

        return allocation;
    }

    void VulkanAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation) const
    {
        vmaDestroyBuffer(m_Allocator, buffer, allocation);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Image
    ////////////////////////////////////////////////////////////////////////////////////
    VmaAllocation VulkanAllocator::CreateImage(VmaMemoryUsage memUsage, VkImage& image, VkImageType type, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t arrayLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkSampleCountFlags samples, VkMemoryPropertyFlags requiredFlags) const
    {
        NG_PROFILE("VkAllocator::AllocateImage()");

        NG_ASSERT(m_Allocator, "[VkAllocator] Allocator not initialized.");
        NG_ASSERT((width > 0) && (height > 0), "[VkAllocator] Invalid width or height passed in for image allocation.");

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = type;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = depth;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = arrayLevels;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = static_cast<VkSampleCountFlagBits>(samples);
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Note: On most modern system all queue family indices are the same for graphics & compute, so we can use exclusive sharing mode between these queue's (since they are the same). If you need to use different queue families, then you need to use concurrent sharing mode and specify the queue family indices in the pQueueFamilyIndices field.

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = memUsage;
        allocCreateInfo.requiredFlags = requiredFlags;

        VmaAllocation allocation = VK_NULL_HANDLE;
        VK_VERIFY(vmaCreateImage(m_Allocator, &imageInfo, &allocCreateInfo, &image, &allocation, nullptr));

        return allocation;
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
    void VulkanAllocator::MapMemory(VmaAllocation allocation, void*& mapData) const
    {
        NG_PROFILE("VkAllocator::MapMemory()");

		NG_ASSERT(m_Allocator, "[VkAllocator] Allocator not initialized.");
		NG_ASSERT((allocation != VK_NULL_HANDLE), "[VkAllocator] Invalid allocation passed in.");

        vmaMapMemory(m_Allocator, allocation, &mapData);
    }

    void VulkanAllocator::UnmapMemory(VmaAllocation allocation) const
    {
        NG_PROFILE("VkAllocator::MapMemory()");

        NG_ASSERT(m_Allocator, "[VkAllocator] Allocator not initialized.");
		NG_ASSERT((allocation != VK_NULL_HANDLE), "[VkAllocator] Invalid allocation passed in.");

        vmaUnmapMemory(m_Allocator, allocation);
    }

    void VulkanAllocator::SetData(VmaAllocation allocation, void* data, size_t size) const
    {
        NG_PROFILE("VkAllocator::SetData()");

		NG_ASSERT((allocation != VK_NULL_HANDLE), "[VkAllocator] Invalid allocation passed in.");
		NG_ASSERT((data != nullptr), "[VkAllocator] Invalid data passed in.");
		NG_ASSERT((size > 0), "[VkAllocator] Invalid size passed in for data.");

        void* mappedData;

        VulkanAllocator::MapMemory(allocation, mappedData);
        VulkanAllocator::SetMappedData(mappedData, data, size);
        VulkanAllocator::UnmapMemory(allocation);
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