#include "ngpch.h"
#include "VulkanDevices.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"

#include <ranges>

namespace
{

	////////////////////////////////////////////////////////////////////////////////////
	// Requested Vulkan features
	////////////////////////////////////////////////////////////////////////////////////
    inline constexpr static VkPhysicalDeviceFeatures s_RequestedDeviceFeatures = {
        .robustBufferAccess = VK_FALSE,
        .fullDrawIndexUint32 = VK_FALSE,
        .imageCubeArray = VK_FALSE,
        .independentBlend = VK_FALSE,
        .geometryShader = VK_FALSE,
        .tessellationShader = VK_FALSE,
        .sampleRateShading = VK_FALSE,
        .dualSrcBlend = VK_FALSE,
        .logicOp = VK_FALSE,
        .multiDrawIndirect = VK_FALSE,
        .drawIndirectFirstInstance = VK_FALSE,
        .depthClamp = VK_FALSE,
        .depthBiasClamp = VK_FALSE,
        .fillModeNonSolid = VK_TRUE, // Needed
        .depthBounds = VK_FALSE,
        .wideLines = VK_FALSE, // Note: Disabled for compatibility reasons
        .largePoints = VK_FALSE,
        .alphaToOne = VK_FALSE,
        .multiViewport = VK_FALSE,
        .samplerAnisotropy = VK_TRUE, // Needed
        .textureCompressionETC2 = VK_FALSE,
        .textureCompressionASTC_LDR = VK_FALSE,
        .textureCompressionBC = VK_FALSE,
        .occlusionQueryPrecise = VK_FALSE,
        .pipelineStatisticsQuery = VK_FALSE,
        .vertexPipelineStoresAndAtomics = VK_FALSE,
        .fragmentStoresAndAtomics = VK_FALSE,
        .shaderTessellationAndGeometryPointSize = VK_FALSE,
        .shaderImageGatherExtended = VK_FALSE,
        .shaderStorageImageExtendedFormats = VK_FALSE,
        .shaderStorageImageMultisample = VK_FALSE,
        .shaderStorageImageReadWithoutFormat = VK_FALSE,
        .shaderStorageImageWriteWithoutFormat = VK_FALSE,
        .shaderUniformBufferArrayDynamicIndexing = VK_FALSE,
        .shaderSampledImageArrayDynamicIndexing = VK_FALSE,
        .shaderStorageBufferArrayDynamicIndexing = VK_FALSE,
        .shaderStorageImageArrayDynamicIndexing = VK_FALSE,
        .shaderClipDistance = VK_FALSE,
        .shaderCullDistance = VK_FALSE,
        .shaderFloat64 = VK_FALSE,
        .shaderInt64 = VK_FALSE,
        .shaderInt16 = VK_FALSE,
        .shaderResourceResidency = VK_FALSE,
        .shaderResourceMinLod = VK_FALSE,
        .sparseBinding = VK_FALSE,
        .sparseResidencyBuffer = VK_FALSE,
        .sparseResidencyImage2D = VK_FALSE,
        .sparseResidencyImage3D = VK_FALSE,
        .sparseResidency2Samples = VK_FALSE,
        .sparseResidency4Samples = VK_FALSE,
        .sparseResidency8Samples = VK_FALSE,
        .sparseResidency16Samples = VK_FALSE,
        .sparseResidencyAliased = VK_FALSE,
        .variableMultisampleRate = VK_FALSE,
        .inheritedQueries = VK_FALSE
    };

    inline constexpr static VkPhysicalDeviceDescriptorIndexingFeatures s_RequestedDescriptorIndexingFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
        .pNext = nullptr,

        .shaderInputAttachmentArrayDynamicIndexing = VK_FALSE,
        .shaderUniformTexelBufferArrayDynamicIndexing = VK_FALSE,
        .shaderStorageTexelBufferArrayDynamicIndexing = VK_FALSE,
        .shaderUniformBufferArrayNonUniformIndexing = VK_FALSE,
        .shaderSampledImageArrayNonUniformIndexing = VK_FALSE,
        .shaderStorageBufferArrayNonUniformIndexing = VK_FALSE,
        .shaderStorageImageArrayNonUniformIndexing = VK_FALSE,
        .shaderInputAttachmentArrayNonUniformIndexing = VK_FALSE,
        .shaderUniformTexelBufferArrayNonUniformIndexing = VK_FALSE,
        .shaderStorageTexelBufferArrayNonUniformIndexing = VK_FALSE,
        .descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE,
        .descriptorBindingSampledImageUpdateAfterBind = VK_FALSE, // Needed for bindless
        .descriptorBindingStorageImageUpdateAfterBind = VK_FALSE,
        .descriptorBindingStorageBufferUpdateAfterBind = VK_FALSE,
        .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_FALSE,
        .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_FALSE,
        .descriptorBindingUpdateUnusedWhilePending = VK_FALSE,
        .descriptorBindingPartiallyBound = VK_FALSE, // Needed for bindless
        .descriptorBindingVariableDescriptorCount = VK_FALSE, // Needed for bindless
        .runtimeDescriptorArray = VK_FALSE, // Needed for bindless
    };

}

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    bool QueueFamilyInfo::SupportsRequired() const
    {
        return ((static_cast<bool>(Flags & QueueFamilyFlags::Graphics)) && (static_cast<bool>(Flags & QueueFamilyFlags::Compute)) && (static_cast<bool>(Flags & QueueFamilyFlags::Present)));
    }

    bool QueueFamilyInfo::EnoughQueues() const
    {
        return Count >= 3;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    bool QueueFamilyIndices::IsComplete() const 
    { 
        return CompletedQueues; 
    }

    bool QueueFamilyIndices::SameQueue() const 
    {
        return ((GraphicsQueue == PresentQueue) && (PresentQueue == ComputeQueue)); 
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Internal structs
    ////////////////////////////////////////////////////////////////////////////////////
    QueueFamilyIndices QueueFamilyIndices::Find(VkSurfaceKHR surface, VkPhysicalDevice device)
	{
        uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        QueueFamilyIndices indices = {};
        indices.Queues.resize(queueFamilyCount);

        // Create into readable format
		for (uint32_t i = 0; i < queueFamilyCount; i++)
		{
            auto& queueFamily = queueFamilies[i];
            QueueFamilyInfo& info = indices.Queues[i];
            info.Index = i;
            info.Count = queueFamily.queueCount;
            info.Flags = static_cast<QueueFamilyFlags>(queueFamily.queueFlags);

			VkBool32 presentSupport;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport)
                info.Flags |= QueueFamilyFlags::Present;
		}

        // Make choices
        for (const auto& queue : indices.Queues) // Note: We want all queues to be from the same queue family to avoid messy synchronization
        {
            if (queue.SupportsRequired())
            {
                if (queue.EnoughQueues())
                {
                    indices.GraphicsQueue = 0;
                    indices.PresentQueue = 1;
                    indices.ComputeQueue = 2;
                }
                else
                {
                    indices.GraphicsQueue = 0;
                    indices.PresentQueue = indices.GraphicsQueue;
                    indices.ComputeQueue = indices.GraphicsQueue;
                }

                indices.QueueFamily = queue.Index;
                indices.CompletedQueues = true;
            }
        }

        NG_ASSERT(indices.CompletedQueues, "[VkDevice] Failed to query queues. Contact developer.");

		return indices;
	}

	SwapChainSupportDetails SwapChainSupportDetails::Query(VkSurfaceKHR surface, VkPhysicalDevice device)
	{
        SwapChainSupportDetails details = {};

		// Capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);

		// Formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        NG_ASSERT((formatCount != 0), "[VkPhysicalDevice] GPU doesn't support any formats?");
		details.Formats.resize(static_cast<size_t>(formatCount));
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.Formats.data());

		// Presentation modes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        NG_ASSERT((formatCount != 0), "[VkPhysicalDevice] GPU doesn't support any presentmodes?");
		details.PresentModes.resize(static_cast<size_t>(presentModeCount));
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.PresentModes.data());

		return details;
	}

    ////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanPhysicalDevice::VulkanPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, std::span<const char*> extensions)
    {
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        NG_ASSERT((deviceCount != 0), "[VkPhysicalDevice] Failed to find a GPU with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) 
		{
            if (PhysicalDeviceSuitable(surface, device, extensions)) 
			{
                m_PhysicalDevice = device;
                break;
            }
        }

        NG_ASSERT(m_PhysicalDevice, "[VkPhysicalDevice] Failed to find a GPU with support for this application's required Vulkan capabilities!");
    }

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
    ////////////////////////////////////////////////////////////////////////////////////
    VkFormat VulkanPhysicalDevice::FindDepthFormat() const
    {
        return FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT
        );
    }

    VkFormat VulkanPhysicalDevice::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
    {
        for (const auto& format : candidates) 
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                return format;
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                return format;
        }

        NG_LOG_ERROR("[VkPhysicalDevice] Failed to find supported format!");
        return VK_FORMAT_UNDEFINED;
    }

	////////////////////////////////////////////////////////////////////////////////////
    // Private methods
	////////////////////////////////////////////////////////////////////////////////////
	bool VulkanPhysicalDevice::PhysicalDeviceSuitable(VkSurfaceKHR surface, VkPhysicalDevice device, std::span<const char*> extensions)
	{
		QueueFamilyIndices indices = QueueFamilyIndices::Find(surface, device);

		bool extensionsSupported = ExtensionsSupported(device, extensions);
		bool swapChainAdequate = false;

		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = SwapChainSupportDetails::Query(surface, device);
			swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
		}

        VkPhysicalDeviceFeatures supportedFeatures = {};
		vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

		// Index features
        VkPhysicalDeviceDescriptorIndexingFeatures indexFeatures = {};
		indexFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		indexFeatures.pNext = nullptr;

        VkPhysicalDeviceFeatures2 deviceFeatures = {};
		deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		deviceFeatures.pNext = &indexFeatures;

		vkGetPhysicalDeviceFeatures2(device, &deviceFeatures);

		return indices.IsComplete() && extensionsSupported && swapChainAdequate && FeaturesSupported(s_RequestedDeviceFeatures, supportedFeatures) && FeaturesSupported(s_RequestedDescriptorIndexingFeatures, indexFeatures);
	}

	bool VulkanPhysicalDevice::ExtensionsSupported(VkPhysicalDevice device, std::span<const char*> extensions)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

		for (const auto& extension : availableExtensions)
			requiredExtensions.erase(extension.extensionName);

		// Note: It's empty if all the required extensions are available
		return requiredExtensions.empty();
	}

	bool VulkanPhysicalDevice::FeaturesSupported(const VkPhysicalDeviceFeatures& requested, const VkPhysicalDeviceFeatures& found)
    {
        constexpr auto features = std::tuple{ 
			&VkPhysicalDeviceFeatures::robustBufferAccess,
            &VkPhysicalDeviceFeatures::fullDrawIndexUint32,
            &VkPhysicalDeviceFeatures::imageCubeArray,
            &VkPhysicalDeviceFeatures::independentBlend,
            &VkPhysicalDeviceFeatures::geometryShader,
            &VkPhysicalDeviceFeatures::tessellationShader,
            &VkPhysicalDeviceFeatures::sampleRateShading,
            &VkPhysicalDeviceFeatures::dualSrcBlend,
            &VkPhysicalDeviceFeatures::logicOp,
            &VkPhysicalDeviceFeatures::multiDrawIndirect,
            &VkPhysicalDeviceFeatures::drawIndirectFirstInstance,
            &VkPhysicalDeviceFeatures::depthClamp,
            &VkPhysicalDeviceFeatures::depthBiasClamp,
            &VkPhysicalDeviceFeatures::fillModeNonSolid,
            &VkPhysicalDeviceFeatures::depthBounds,
            &VkPhysicalDeviceFeatures::wideLines,
            &VkPhysicalDeviceFeatures::largePoints,
            &VkPhysicalDeviceFeatures::alphaToOne,
            &VkPhysicalDeviceFeatures::multiViewport,
            &VkPhysicalDeviceFeatures::samplerAnisotropy,
            &VkPhysicalDeviceFeatures::textureCompressionETC2,
            &VkPhysicalDeviceFeatures::textureCompressionASTC_LDR,
            &VkPhysicalDeviceFeatures::textureCompressionBC,
            &VkPhysicalDeviceFeatures::occlusionQueryPrecise,
            &VkPhysicalDeviceFeatures::pipelineStatisticsQuery,
            &VkPhysicalDeviceFeatures::vertexPipelineStoresAndAtomics,
            &VkPhysicalDeviceFeatures::fragmentStoresAndAtomics,
            &VkPhysicalDeviceFeatures::shaderTessellationAndGeometryPointSize,
            &VkPhysicalDeviceFeatures::shaderImageGatherExtended,
            &VkPhysicalDeviceFeatures::shaderStorageImageExtendedFormats,
            &VkPhysicalDeviceFeatures::shaderStorageImageMultisample,
            &VkPhysicalDeviceFeatures::shaderStorageImageReadWithoutFormat,
            &VkPhysicalDeviceFeatures::shaderStorageImageWriteWithoutFormat,
            &VkPhysicalDeviceFeatures::shaderUniformBufferArrayDynamicIndexing,
            &VkPhysicalDeviceFeatures::shaderSampledImageArrayDynamicIndexing,
            &VkPhysicalDeviceFeatures::shaderStorageBufferArrayDynamicIndexing,
            &VkPhysicalDeviceFeatures::shaderStorageImageArrayDynamicIndexing,
            &VkPhysicalDeviceFeatures::shaderClipDistance,
            &VkPhysicalDeviceFeatures::shaderCullDistance,
            &VkPhysicalDeviceFeatures::shaderFloat64,
            &VkPhysicalDeviceFeatures::shaderInt64,
            &VkPhysicalDeviceFeatures::shaderInt16,
            &VkPhysicalDeviceFeatures::shaderResourceResidency,
            &VkPhysicalDeviceFeatures::shaderResourceMinLod,
            &VkPhysicalDeviceFeatures::sparseBinding,
            &VkPhysicalDeviceFeatures::sparseResidencyBuffer,
            &VkPhysicalDeviceFeatures::sparseResidencyImage2D,
            &VkPhysicalDeviceFeatures::sparseResidencyImage3D,
            &VkPhysicalDeviceFeatures::sparseResidency2Samples,
            &VkPhysicalDeviceFeatures::sparseResidency4Samples,
            &VkPhysicalDeviceFeatures::sparseResidency8Samples,
            &VkPhysicalDeviceFeatures::sparseResidency16Samples,
            &VkPhysicalDeviceFeatures::sparseResidencyAliased,
            &VkPhysicalDeviceFeatures::variableMultisampleRate,
            &VkPhysicalDeviceFeatures::inheritedQueries 
		};

        // Use std::apply to iterate over the tuple
        bool failed = false;
        std::apply([&](auto... featurePtr) { ((failed |= (requested.*featurePtr && !(found.*featurePtr))), ...); }, features);

        return !failed;
    }

    bool VulkanPhysicalDevice::FeaturesSupported(const VkPhysicalDeviceDescriptorIndexingFeatures& requested, const VkPhysicalDeviceDescriptorIndexingFeatures& found)
    {
        constexpr auto features = std::tuple{
            &VkPhysicalDeviceDescriptorIndexingFeatures::shaderInputAttachmentArrayDynamicIndexing,
            &VkPhysicalDeviceDescriptorIndexingFeatures::shaderUniformTexelBufferArrayDynamicIndexing,
            &VkPhysicalDeviceDescriptorIndexingFeatures::shaderStorageTexelBufferArrayDynamicIndexing,
            &VkPhysicalDeviceDescriptorIndexingFeatures::shaderUniformBufferArrayNonUniformIndexing,
            &VkPhysicalDeviceDescriptorIndexingFeatures::shaderSampledImageArrayNonUniformIndexing,
            &VkPhysicalDeviceDescriptorIndexingFeatures::shaderStorageBufferArrayNonUniformIndexing,
            &VkPhysicalDeviceDescriptorIndexingFeatures::shaderStorageImageArrayNonUniformIndexing,
            &VkPhysicalDeviceDescriptorIndexingFeatures::shaderInputAttachmentArrayNonUniformIndexing,
            &VkPhysicalDeviceDescriptorIndexingFeatures::shaderUniformTexelBufferArrayNonUniformIndexing,
            &VkPhysicalDeviceDescriptorIndexingFeatures::shaderStorageTexelBufferArrayNonUniformIndexing,
            &VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingUniformBufferUpdateAfterBind,
            &VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingSampledImageUpdateAfterBind,
            &VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingStorageImageUpdateAfterBind,
            &VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingStorageBufferUpdateAfterBind,
            &VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingUniformTexelBufferUpdateAfterBind,
            &VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingStorageTexelBufferUpdateAfterBind,
            &VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingUpdateUnusedWhilePending,
            &VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingPartiallyBound,
            &VkPhysicalDeviceDescriptorIndexingFeatures::descriptorBindingVariableDescriptorCount,
            &VkPhysicalDeviceDescriptorIndexingFeatures::runtimeDescriptorArray,
        };

        // Use std::apply to iterate over the tuple
        bool failed = false;
        std::apply([&](auto... featurePtr) { ((failed |= (requested.*featurePtr && !(found.*featurePtr))), ...); }, features);

        return !failed;
    }

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
    VulkanLogicalDevice::VulkanLogicalDevice(VkSurfaceKHR surface, VulkanPhysicalDevice& physicalDevice, std::span<const char*> extensions)
		: m_PhysicalDevice(physicalDevice)
	{
		QueueFamilyIndices indices = QueueFamilyIndices::Find(surface, m_PhysicalDevice.GetVkPhysicalDevice());

        uint32_t queueCount = (indices.SameQueue() ? 1 : 3);
        std::vector<float> queuePriorities(queueCount, 1.0f);

        VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.QueueFamily;
		queueCreateInfo.queueCount = queueCount;
		queueCreateInfo.pQueuePriorities = queuePriorities.data();

		// Enable dynamic rendering features // Note: Dynamic rendering & bindless is currently disabled
		// VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature = {};
		// dynamicRenderingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
		// dynamicRenderingFeature.dynamicRendering = VK_TRUE;
        
		VkPhysicalDeviceDescriptorIndexingFeaturesEXT indexingFeatures = s_RequestedDescriptorIndexingFeatures;
        indexingFeatures.pNext = nullptr; //&dynamicRenderingFeature;

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &indexingFeatures; // Chain indexing
		createInfo.queueCreateInfoCount = 1;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.pEnabledFeatures = &s_RequestedDeviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if constexpr (VulkanContext::Validation)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(VulkanContext::ValidationLayers.size());
			createInfo.ppEnabledLayerNames = VulkanContext::ValidationLayers.data();
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		VK_VERIFY(vkCreateDevice(m_PhysicalDevice.GetVkPhysicalDevice(), &createInfo, nullptr, &m_LogicalDevice));

		// Retrieve the graphics/compute/present queue handle
        {
            VkDeviceQueueInfo2 queueInfo = {};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
            queueInfo.queueFamilyIndex = indices.QueueFamily;
            queueInfo.flags = 0; // Note: Should always be 0

            queueInfo.queueIndex = indices.GraphicsQueue;
            vkGetDeviceQueue2(m_LogicalDevice, &queueInfo, &m_GraphicsQueue);

            queueInfo.queueIndex = indices.ComputeQueue;
            vkGetDeviceQueue2(m_LogicalDevice, &queueInfo, &m_ComputeQueue);

            queueInfo.queueIndex = indices.PresentQueue;
            vkGetDeviceQueue2(m_LogicalDevice, &queueInfo, &m_PresentQueue);
        }
	}

    VulkanLogicalDevice::~VulkanLogicalDevice()
	{
		vkDestroyDevice(m_LogicalDevice, nullptr);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void VulkanLogicalDevice::Wait() const
	{
        VK_VERIFY(vkDeviceWaitIdle(m_LogicalDevice));
	}

}