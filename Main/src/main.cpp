#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <shaderc/shaderc.hpp>

#include <vulkan/vulkan.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <array>
#include <set>
#include <numeric>

// Vulkan Surface
#if defined(_WIN32) || defined(_WIN64)
    #define VK_KHR_SURFACE_TYPE_NAME "VK_KHR_win32_surface"
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_MAC
        #define VK_KHR_SURFACE_TYPE_NAME "VK_EXT_metal_surface"
    #else
        #error Obsidian Vulkan: Unsupported Apple platform...
    #endif
#elif defined(__linux__)
    #define VK_KHR_SURFACE_TYPE_NAME "VK_KHR_wayland_surface"
#else
    #error Obsidian Vulkan: Unsupported platform...
#endif

// Settings
inline static constexpr const bool s_EnableValidationLayers = true;

inline static constexpr const uint8_t s_MaxFramesInFlight = 3;

inline static constexpr const auto s_ValidationLayers = std::to_array<const char*>({
    "VK_LAYER_KHRONOS_validation",
    "VK_LAYER_KHRONOS_synchronization2"
});

inline static constexpr const auto s_DeviceExtensions = std::to_array<const char*>({
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
});

// External vulkan functions
static VkResult CreateDebugUtilsMessengerEXT(VkInstance m_Instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) 
        return func(m_Instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(VkInstance m_Instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) 
        func(m_Instance, debugMessenger, pAllocator);
}

// Vulkan helper functions
static bool CheckValidationLayerSupport() 
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : s_ValidationLayers) 
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (std::strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}


// Vulkan helper structs
struct QueueFamilyIndices 
{
public:
    std::optional<uint32_t> GraphicsFamily;
    std::optional<uint32_t> PresentFamily;

public:
    inline bool IsComplete() { return (GraphicsFamily.has_value() && PresentFamily.has_value()); }
};

struct SwapchainSupportDetails 
{
public:
    VkSurfaceCapabilitiesKHR Capabilities;
    std::vector<VkSurfaceFormatKHR> Formats;
    std::vector<VkPresentModeKHR> PresentModes;
};

struct Vertex 
{
public:
    glm::vec2 Position;
    glm::vec3 Colour;

public:
    static VkVertexInputBindingDescription GetBindingDescription() 
	{
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() 
	{
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, Position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, Colour);

        return attributeDescriptions;
    }
};

// Vertices & Indices
inline static constexpr const auto s_Vertices = std::to_array<Vertex>({
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
});

inline static constexpr const auto s_Indices = std::to_array<uint16_t>({
    0, 1, 2, 
	2, 3, 0
});

// Application class
class Application
{
public:
	Application()
	{
		InitWindow();
		InitVulkan();
		InitResources();
	}
	~Application()
	{
		vkDeviceWaitIdle(m_LogicalDevice);

		for (auto framebuffer : m_Framebuffers) 
            vkDestroyFramebuffer(m_LogicalDevice, framebuffer, nullptr);

        for (auto imageView : m_ImageViews)
            vkDestroyImageView(m_LogicalDevice, imageView, nullptr);

        vkDestroySwapchainKHR(m_LogicalDevice, m_Swapchain, nullptr);

		vkDestroyPipeline(m_LogicalDevice, m_Pipeline, nullptr);
        vkDestroyPipelineLayout(m_LogicalDevice, m_PipelineLayout, nullptr);

        vkDestroyRenderPass(m_LogicalDevice, m_Renderpass1, nullptr);
		vkDestroyRenderPass(m_LogicalDevice, m_Renderpass0, nullptr);

        vkDestroyBuffer(m_LogicalDevice, m_IndexBuffer, nullptr);
        vkFreeMemory(m_LogicalDevice, m_IndexBufferMemory, nullptr);

        vkDestroyBuffer(m_LogicalDevice, m_VertexBuffer, nullptr);
        vkFreeMemory(m_LogicalDevice, m_VertexBufferMemory, nullptr);

		for (auto renderFinishedSemaphore : renderFinishedSemaphores)
			vkDestroySemaphore(m_LogicalDevice, renderFinishedSemaphore, nullptr);

        for (size_t i = 0; i < s_MaxFramesInFlight; i++) 
		{
            vkDestroySemaphore(m_LogicalDevice, m_ImageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_LogicalDevice, m_InFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);

        vkDestroyDevice(m_LogicalDevice, nullptr);

        if constexpr (s_EnableValidationLayers)
            DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);

        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        vkDestroyInstance(m_Instance, nullptr);

        glfwDestroyWindow(m_Window);

        glfwTerminate();
	}

public:
	void Run()
	{
		while (!glfwWindowShouldClose(m_Window)) 
		{
            glfwPollEvents();

			// TODO: Draw frame
        }
	}

private:
	GLFWwindow* m_Window = nullptr;

    VkInstance m_Instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_LogicalDevice = VK_NULL_HANDLE;

    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VkQueue m_PresentQueue = VK_NULL_HANDLE;

    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> m_Images = { };
    std::vector<VkImageView> m_ImageViews = { };
    std::vector<VkFramebuffer> m_Framebuffers = { };

    VkRenderPass m_Renderpass0 = VK_NULL_HANDLE;
    VkRenderPass m_Renderpass1 = VK_NULL_HANDLE;
	
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_Pipeline = VK_NULL_HANDLE;

    VkCommandPool m_CommandPool = VK_NULL_HANDLE;

	std::array<VkCommandBuffer, static_cast<size_t>(s_MaxFramesInFlight)> m_CommandBuffers = { };

	VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;
	VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_IndexBufferMemory = VK_NULL_HANDLE;

    std::array<VkSemaphore, static_cast<size_t>(s_MaxFramesInFlight)> m_ImageAvailableSemaphores = { };
    std::vector<VkSemaphore> renderFinishedSemaphores = { };
	VkSemaphore m_TimelineSemaphore = VK_NULL_HANDLE;
	uint64_t m_CurrentTimelineValue = 0;
    std::array<VkFence, static_cast<size_t>(s_MaxFramesInFlight)> m_InFlightFences = { };

	uint32_t m_CurrentFrame = 0;
	uint32_t m_AcquiredImage = 0;

private:
	void InitWindow()
	{
		glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        m_Window = glfwCreateWindow(1280, 720, "Minimal queuePresent error", nullptr, nullptr);
        glfwSetWindowUserPointer(m_Window, this);
	}

	void InitVulkan()
	{
		CreateInstance();
        SetupDebugMessenger();
        CreateSurface();
        ChoosePhysicalDevice();
        CreateLogicalDevice();
        CreateSwapchain();
        CreateImageViews();
	}

	void InitResources()
	{
		// CreateRenderPass0();
		// CreateFramebuffers0();
		// CreateRenderPass1();
		// CreateFramebuffers1();

        // CreateGraphicsPipeline();
        // CreateCommandPool();
        // CreateVertexBuffer();
        // CreateIndexBuffer();
        // CreateCommandBuffers();
        // CreateSyncObjects();
	}

private:
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
	{
		(void)messageSeverity; (void)messageType; (void)pUserData;

        std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
	}

	std::vector<const char*> GetRequiredExtensions() 
	{
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if constexpr (s_EnableValidationLayers) 
		{
			// TODO: Push all extensions
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) 
	{
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = &DebugCallback;
    }

	bool CheckDeviceExtensionSupport(VkPhysicalDevice device) 
	{
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(s_DeviceExtensions.begin(), s_DeviceExtensions.end());

        for (const auto& extension : availableExtensions)
            requiredExtensions.erase(extension.extensionName);

        return requiredExtensions.empty();
    }

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) 
	{
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) 
		{
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
                indices.GraphicsFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);

            if (presentSupport) 
                indices.PresentFamily = i;

            if (indices.IsComplete()) 
                break;

            i++;
        }

        return indices;
    }

	SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device) 
	{
        SwapchainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.Capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

        if (formatCount != 0) 
		{
            details.Formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.Formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
		{
            details.PresentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.PresentModes.data());
        }

        return details;
    }

	bool IsDeviceSuitable(VkPhysicalDevice device) 
	{
        QueueFamilyIndices indices = FindQueueFamilies(device);

        bool extensionsSupported = CheckDeviceExtensionSupport(device);

        bool swapchainAdequate = false;
        if (extensionsSupported) 
		{
            SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(device);
            swapchainAdequate = !swapchainSupport.Formats.empty() && !swapchainSupport.PresentModes.empty();
        }

        return indices.IsComplete() && extensionsSupported && swapchainAdequate;
    }

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) 
	{
        for (const auto& availableFormat : availableFormats) 
		{
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;
        }

        return availableFormats[0];
    }

    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) 
	{
        for (const auto& availablePresentMode : availablePresentModes)
		{
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
	{
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
		{
            return capabilities.currentExtent;
        } 
		else
		{
            int width, height;
            glfwGetFramebufferSize(m_Window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

private:
	void CreateInstance()
	{
		if (s_EnableValidationLayers && !CheckValidationLayerSupport()) 
            throw std::runtime_error("Validation layers requested, but not available!");

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Minimal vkQueuePresent issue";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 3, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
		if constexpr (s_EnableValidationLayers) 
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(s_ValidationLayers.size());
			createInfo.ppEnabledLayerNames = s_ValidationLayers.data();

			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        } 
		else 
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
            throw std::runtime_error("Failed to create instance!");
	}

	void SetupDebugMessenger()
	{
		if constexpr (!s_EnableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
            throw std::runtime_error("Failed to set up debug messenger!");
	}

	void CreateSurface() 
	{
        if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS)
            throw std::runtime_error("Failed to create window surface!");
    }

	void ChoosePhysicalDevice() 
	{
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

        if (deviceCount == 0)
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

        for (const auto& device : devices) 
		{
            if (IsDeviceSuitable(device)) 
			{
                m_PhysicalDevice = device;
                break;
            }
        }

        if (m_PhysicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("Failed to find a suitable GPU!");
    }

	void CreateLogicalDevice() 
	{
        QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.GraphicsFamily.value(), indices.PresentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) 
		{
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(s_DeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = s_DeviceExtensions.data();

		if constexpr (s_EnableValidationLayers) 
		{
            createInfo.enabledLayerCount = static_cast<uint32_t>(s_ValidationLayers.size());
            createInfo.ppEnabledLayerNames = s_ValidationLayers.data();
        }
		else 
		{
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_LogicalDevice) != VK_SUCCESS) 
            throw std::runtime_error("failed to create logical device!");

        vkGetDeviceQueue(m_LogicalDevice, indices.GraphicsFamily.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_LogicalDevice, indices.PresentFamily.value(), 0, &m_PresentQueue);
    }

	void CreateSwapchain() 
	{
        SwapchainSupportDetails swapchainSupport = QuerySwapchainSupport(m_PhysicalDevice);

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapchainSupport.Formats);
        VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapchainSupport.PresentModes);
        VkExtent2D extent = ChooseSwapExtent(swapchainSupport.Capabilities);

        uint32_t imageCount = swapchainSupport.Capabilities.minImageCount + 1;
        if (swapchainSupport.Capabilities.maxImageCount > 0 && imageCount > swapchainSupport.Capabilities.maxImageCount)
            imageCount = swapchainSupport.Capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_Surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);
        uint32_t queueFamilyIndices[] = {indices.GraphicsFamily.value(), indices.PresentFamily.value()};

        if (indices.GraphicsFamily != indices.PresentFamily) 
		{
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else 
		{
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapchainSupport.Capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(m_LogicalDevice, &createInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
            throw std::runtime_error("Failed to create swap chain!");

        vkGetSwapchainImagesKHR(m_LogicalDevice, m_Swapchain, &imageCount, nullptr);
        m_Images.resize(imageCount);
        vkGetSwapchainImagesKHR(m_LogicalDevice, m_Swapchain, &imageCount, m_Images.data());
    }

	void CreateImageViews() 
	{
        m_ImageViews.resize(m_Images.size());

        for (size_t i = 0; i < m_Images.size(); i++) 
		{
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_Images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = ChooseSwapSurfaceFormat(QuerySwapchainSupport(m_PhysicalDevice).Formats).format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(m_LogicalDevice, &createInfo, nullptr, &m_ImageViews[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create image views!");
        }
    }

};

int main(int argc, char* argv[])
{
	(void)argc; (void)argv;

	Application app;
	app.Run();

	return 0;
}
