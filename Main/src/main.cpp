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
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,

	#if defined(__APPLE__)
	"VK_KHR_portability_subset",
	#endif


	"VK_KHR_synchronization2",
	"VK_KHR_copy_commands2"
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

// Shaders
inline static constexpr const std::string_view s_VertexShader = R"(
#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
)";

inline static constexpr const std::string_view s_FragmentShader = R"(
#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
)";

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

		for (auto framebuffer : m_Framebuffers0) 
            vkDestroyFramebuffer(m_LogicalDevice, framebuffer, nullptr);
		for (auto framebuffer : m_Framebuffers1) 
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

		vkDestroySemaphore(m_LogicalDevice, m_TimelineSemaphore, nullptr);

		for (auto renderFinishedSemaphore : m_RenderFinishedSemaphores)
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

			// Draw frame
			{
				vkWaitForFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

				VkResult result = vkAcquireNextImageKHR(m_LogicalDevice, m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &m_AcquiredImage);

				// if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				// 	recreateSwapChain();
				// 	return;
				// } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
				// 	throw std::runtime_error("failed to acquire swap chain image!");
				// }

				vkResetFences(m_LogicalDevice, 1, &m_InFlightFences[m_CurrentFrame]);

				// TODO: Maybe reset command pool to mimic Obsidian
				vkResetCommandBuffer(m_CommandBuffers0[m_CurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);
				vkResetCommandBuffer(m_CommandBuffers1[m_CurrentFrame], /*VkCommandBufferResetFlagBits*/ 0);
				
				// commandBuffer 0 
				{
					{
						VkCommandBufferBeginInfo beginInfo = {};
						beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

						if (vkBeginCommandBuffer(m_CommandBuffers0[m_CurrentFrame], &beginInfo) != VK_SUCCESS)
							throw std::runtime_error("Failed to begin command buffer 0!");

						VkRenderPassBeginInfo renderpassInfo = {};
						renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
						renderpassInfo.renderPass = m_Renderpass0;
						renderpassInfo.framebuffer = m_Framebuffers0[m_CurrentFrame];
						renderpassInfo.renderArea.offset = { 0, 0 };
						renderpassInfo.renderArea.extent = { ChooseSwapExtent(QuerySwapchainSupport(m_PhysicalDevice).Capabilities).width, ChooseSwapExtent(QuerySwapchainSupport(m_PhysicalDevice).Capabilities).height };

						// Clear values
						VkClearValue colorClear = VkClearValue({ 0.0f, 0.0f, 0.0f, 1.0f }); 

						renderpassInfo.clearValueCount = 1;
						renderpassInfo.pClearValues = &colorClear;

						VkSubpassBeginInfo subpassInfo = {};
						subpassInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO;
						subpassInfo.contents = VK_SUBPASS_CONTENTS_INLINE;

						vkCmdBeginRenderPass2(m_CommandBuffers0[m_CurrentFrame], &renderpassInfo, &subpassInfo);

						VkViewport vkViewport = {};
						vkViewport.x = 0.0f;
						vkViewport.y = 0.0f;
						vkViewport.width = ChooseSwapExtent(QuerySwapchainSupport(m_PhysicalDevice).Capabilities).width;
						vkViewport.height = ChooseSwapExtent(QuerySwapchainSupport(m_PhysicalDevice).Capabilities).height;
						vkViewport.minDepth = 0.0f;
						vkViewport.maxDepth = 0.0f;
						vkCmdSetViewport(m_CommandBuffers0[m_CurrentFrame], 0, 1, &vkViewport);

						VkRect2D vkScissor = {};
						vkScissor.offset = { 0, 0 };
						vkScissor.extent = { ChooseSwapExtent(QuerySwapchainSupport(m_PhysicalDevice).Capabilities).width, ChooseSwapExtent(QuerySwapchainSupport(m_PhysicalDevice).Capabilities).height  };
						vkCmdSetScissor(m_CommandBuffers0[m_CurrentFrame], 0, 1, &vkScissor);

						// TODO: Do something

						VkSubpassEndInfo endInfo = {};
						endInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO;

						vkCmdEndRenderPass2(m_CommandBuffers0[m_CurrentFrame], &endInfo);
						
						if (vkEndCommandBuffer(m_CommandBuffers0[m_CurrentFrame]) != VK_SUCCESS)
							throw std::runtime_error("Failed to end command buffer 0!");
					}
					
					{
						// Wait for swapchain image
						VkSemaphoreSubmitInfo imageInfo = {};
						imageInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
						imageInfo.semaphore = m_ImageAvailableSemaphores[m_CurrentFrame];
						imageInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
						imageInfo.value = 0ull;

						// Signal timeline 
						VkSemaphoreSubmitInfo timelineInfo = {};
						timelineInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
						timelineInfo.semaphore = m_TimelineSemaphore;
						timelineInfo.value = ++m_CurrentTimelineValue;

						VkCommandBufferSubmitInfo commandInfo = {};
						commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
						commandInfo.commandBuffer = m_CommandBuffers0[m_CurrentFrame];

						// Submit info
						VkSubmitInfo2 submitInfo = {};
						submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

						submitInfo.waitSemaphoreInfoCount = 1u;
						submitInfo.pWaitSemaphoreInfos = &imageInfo;

						submitInfo.commandBufferInfoCount = 1u;
						submitInfo.pCommandBufferInfos = &commandInfo;

						submitInfo.signalSemaphoreInfoCount = 1;
						submitInfo.pSignalSemaphoreInfos = &timelineInfo;

						if (vkQueueSubmit2(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
							throw std::runtime_error("Failed to submit command buffer 0.");
					}
				}

				// commandBuffer 1
				{
					{
						VkCommandBufferBeginInfo beginInfo = {};
						beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

						if (vkBeginCommandBuffer(m_CommandBuffers1[m_CurrentFrame], &beginInfo) != VK_SUCCESS)
							throw std::runtime_error("Failed to begin command buffer 1!");

						VkRenderPassBeginInfo renderpassInfo = {};
						renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
						renderpassInfo.renderPass = m_Renderpass1;
						renderpassInfo.framebuffer = m_Framebuffers1[m_CurrentFrame];
						renderpassInfo.renderArea.offset = { 0, 0 };
						renderpassInfo.renderArea.extent = { ChooseSwapExtent(QuerySwapchainSupport(m_PhysicalDevice).Capabilities).width, ChooseSwapExtent(QuerySwapchainSupport(m_PhysicalDevice).Capabilities).height };

						// Clear values
						VkClearValue colorClear = VkClearValue({ 0.0f, 0.0f, 0.0f, 1.0f }); 

						renderpassInfo.clearValueCount = 1;
						renderpassInfo.pClearValues = &colorClear;

						VkSubpassBeginInfo subpassInfo = {};
						subpassInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO;
						subpassInfo.contents = VK_SUBPASS_CONTENTS_INLINE;

						vkCmdBeginRenderPass2(m_CommandBuffers1[m_CurrentFrame], &renderpassInfo, &subpassInfo);

						VkViewport vkViewport = {};
						vkViewport.x = 0.0f;
						vkViewport.y = 0.0f;
						vkViewport.width = ChooseSwapExtent(QuerySwapchainSupport(m_PhysicalDevice).Capabilities).width;
						vkViewport.height = ChooseSwapExtent(QuerySwapchainSupport(m_PhysicalDevice).Capabilities).height;
						vkViewport.minDepth = 0.0f;
						vkViewport.maxDepth = 0.0f;
						vkCmdSetViewport(m_CommandBuffers1[m_CurrentFrame], 0, 1, &vkViewport);

						VkRect2D vkScissor = {};
						vkScissor.offset = { 0, 0 };
						vkScissor.extent = { ChooseSwapExtent(QuerySwapchainSupport(m_PhysicalDevice).Capabilities).width, ChooseSwapExtent(QuerySwapchainSupport(m_PhysicalDevice).Capabilities).height  };
						vkCmdSetScissor(m_CommandBuffers1[m_CurrentFrame], 0, 1, &vkScissor);

						// TODO: Do something

						VkSubpassEndInfo endInfo = {};
						endInfo.sType = VK_STRUCTURE_TYPE_SUBPASS_END_INFO;

						vkCmdEndRenderPass2(m_CommandBuffers1[m_CurrentFrame], &endInfo);
						
						if (vkEndCommandBuffer(m_CommandBuffers1[m_CurrentFrame]) != VK_SUCCESS)
							throw std::runtime_error("Failed to end command buffer 1!");
					}
					
					{
						// Wait for commandBuffer 0
						VkSemaphoreSubmitInfo waitInfo = {};
						waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
						waitInfo.semaphore = m_TimelineSemaphore;
						waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
						waitInfo.value = m_CurrentTimelineValue;

						// Signal timeline and renderFinishedSemaphore
						std::array<VkSemaphoreSubmitInfo, 2> signalInfos = {};

						VkSemaphoreSubmitInfo& timelineInfo = signalInfos[0];
						timelineInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
						timelineInfo.semaphore = m_TimelineSemaphore;
						timelineInfo.value = ++m_CurrentTimelineValue;

						VkSemaphoreSubmitInfo& info = signalInfos[1];
						info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
						info.semaphore = m_RenderFinishedSemaphores[m_AcquiredImage];
						info.stageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT; // Note: Before a swapchain can be present this stage must be finished
						info.value = 0ull;

						VkCommandBufferSubmitInfo commandInfo = {};
						commandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
						commandInfo.commandBuffer = m_CommandBuffers1[m_CurrentFrame];

						// Submit info
						VkSubmitInfo2 submitInfo = {};
						submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

						submitInfo.waitSemaphoreInfoCount = 1u;
						submitInfo.pWaitSemaphoreInfos = &waitInfo;

						submitInfo.commandBufferInfoCount = 1u;
						submitInfo.pCommandBufferInfos = &commandInfo;

						submitInfo.signalSemaphoreInfoCount = static_cast<uint32_t>(signalInfos.size());
						submitInfo.pSignalSemaphoreInfos = signalInfos.data();

						if (vkQueueSubmit2(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
							throw std::runtime_error("Failed to submit command buffer 0.");
					}

				}

				VkPresentInfoKHR presentInfo = {};
				presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

				presentInfo.waitSemaphoreCount = 1;
				presentInfo.pWaitSemaphores = &m_RenderFinishedSemaphores[m_AcquiredImage];

				VkSwapchainKHR swapChains[] = {m_Swapchain};
				presentInfo.swapchainCount = 1;
				presentInfo.pSwapchains = swapChains;

				presentInfo.pImageIndices = &m_AcquiredImage;

				result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);

				// if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
				// 	framebufferResized = false;
				// 	recreateSwapChain();
				// } else if (result != VK_SUCCESS) {
				// 	throw std::runtime_error("failed to present swap chain image!");
				// }
				
				if (result != VK_SUCCESS)
					throw std::runtime_error("QueuePresent failed!");

				m_CurrentFrame = (m_CurrentFrame + 1) % s_MaxFramesInFlight;		
			}
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

    VkRenderPass m_Renderpass0 = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> m_Framebuffers0 = { };
    VkRenderPass m_Renderpass1 = VK_NULL_HANDLE;
	std::vector<VkFramebuffer> m_Framebuffers1 = { };
	
    VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_Pipeline = VK_NULL_HANDLE;

    VkCommandPool m_CommandPool = VK_NULL_HANDLE;

	std::array<VkCommandBuffer, static_cast<size_t>(s_MaxFramesInFlight)> m_CommandBuffers0 = { };
	std::array<VkCommandBuffer, static_cast<size_t>(s_MaxFramesInFlight)> m_CommandBuffers1 = { };

	VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;
	VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_IndexBufferMemory = VK_NULL_HANDLE;

    std::array<VkSemaphore, static_cast<size_t>(s_MaxFramesInFlight)> m_ImageAvailableSemaphores = { };
    std::vector<VkSemaphore> m_RenderFinishedSemaphores = { };
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
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

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
		CreateSyncObjects();
	}

	void InitResources()
	{
		CreateRenderPass0();
		CreateFramebuffers0();
		CreateRenderPass1();
		CreateFramebuffers1();

		CreateCommandPool();
		CreateCommandBuffers0();
		CreateCommandBuffers1();
        
		CreateGraphicsPipeline();
        CreateVertexBuffer();
        CreateIndexBuffer();
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

	std::vector<uint32_t> CompileShader(std::string_view code, shaderc_shader_kind kind)
	{
		shaderc::Compiler compiler = {};
        shaderc::CompileOptions options = {};
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        
		options.SetSourceLanguage(shaderc_source_language_glsl);
        shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(code.data(), kind, "", "main", options);

        if (module.GetCompilationStatus() != shaderc_compilation_status_success)
			throw std::runtime_error("Failed to compile shader!");

        return std::vector<uint32_t>(module.cbegin(), module.cend());
	}

	VkShaderModule CreateShaderModule(const std::vector<uint32_t>& code) 
	{
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size() * sizeof(uint32_t);
        createInfo.pCode = code.data();

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(m_LogicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
            throw std::runtime_error("Failed to create shader module!");

        return shaderModule;
    }

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_LogicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
            throw std::runtime_error("Failed to create buffer!");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_LogicalDevice, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_LogicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate buffer memory!");

        vkBindBufferMemory(m_LogicalDevice, buffer, bufferMemory, 0);
    }

    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
	{
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_GraphicsQueue);

        vkFreeCommandBuffers(m_LogicalDevice, m_CommandPool, 1, &commandBuffer);
    }

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
	{
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
		{
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        }

        throw std::runtime_error("Failed to find suitable memory type!");
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
		#if defined(__APPLE__)
            extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
			createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        #endif	
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

		VkPhysicalDeviceSynchronization2Features synchronization2Features = {};
        synchronization2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
		synchronization2Features.synchronization2 = VK_TRUE;
        synchronization2Features.pNext = nullptr;

		VkPhysicalDeviceTimelineSemaphoreFeatures timelineFeatures = {};
        timelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
		timelineFeatures.timelineSemaphore = VK_TRUE;
        timelineFeatures.pNext = &synchronization2Features;

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = &timelineFeatures;

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

	void CreateSyncObjects()
	{
        m_RenderFinishedSemaphores.resize(m_Images.size());

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < s_MaxFramesInFlight; i++) 
		{
            if (vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_LogicalDevice, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create synchronization objects for a frame!");
        }

		m_RenderFinishedSemaphores.resize(m_Images.size());
		for (size_t i = 0; i < m_RenderFinishedSemaphores.size(); i++)
		{
			if (vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS)
				throw std::runtime_error("Failed to create synchronization objects for a frame!");
		}

		VkSemaphoreTypeCreateInfo timelineInfo = {};
		timelineInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		timelineInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
		timelineInfo.initialValue = 0;

		semaphoreInfo.pNext = &timelineInfo;

		if (vkCreateSemaphore(m_LogicalDevice, &semaphoreInfo, nullptr, &m_TimelineSemaphore) != VK_SUCCESS)
			throw std::runtime_error("Failed to create synchronization objects for a frame!");
	}

private:
	void CreateRenderPass0() 
	{
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = ChooseSwapSurfaceFormat(QuerySwapchainSupport(m_PhysicalDevice).Formats).format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 0;

        if (vkCreateRenderPass(m_LogicalDevice, &renderPassInfo, nullptr, &m_Renderpass0) != VK_SUCCESS)
            throw std::runtime_error("Failed to create render pass!");
    }

	void CreateRenderPass1() 
	{
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = ChooseSwapSurfaceFormat(QuerySwapchainSupport(m_PhysicalDevice).Formats).format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 0;

        if (vkCreateRenderPass(m_LogicalDevice, &renderPassInfo, nullptr, &m_Renderpass1) != VK_SUCCESS)
            throw std::runtime_error("Failed to create render pass!");
    }

	void CreateFramebuffers0() 
	{
        m_Framebuffers0.resize(m_ImageViews.size());

        for (size_t i = 0; i < m_ImageViews.size(); i++) 
		{
            VkImageView attachments[] = {
                m_ImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_Renderpass0;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = ChooseSwapExtent(QuerySwapchainSupport(m_PhysicalDevice).Capabilities).width;
            framebufferInfo.height = ChooseSwapExtent(QuerySwapchainSupport(m_PhysicalDevice).Capabilities).height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_LogicalDevice, &framebufferInfo, nullptr, &m_Framebuffers0[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create framebuffer!");
        }
    }

	void CreateFramebuffers1() 
	{
        m_Framebuffers1.resize(m_ImageViews.size());

        for (size_t i = 0; i < m_ImageViews.size(); i++) 
		{
            VkImageView attachments[] = {
                m_ImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_Renderpass1;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = ChooseSwapExtent(QuerySwapchainSupport(m_PhysicalDevice).Capabilities).width;
            framebufferInfo.height = ChooseSwapExtent(QuerySwapchainSupport(m_PhysicalDevice).Capabilities).height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_LogicalDevice, &framebufferInfo, nullptr, &m_Framebuffers1[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create framebuffer!");
        }
    }

	void CreateCommandPool() 
	{
        QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_PhysicalDevice);

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();

        if (vkCreateCommandPool(m_LogicalDevice, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
            throw std::runtime_error("Failed to create graphics command pool!");
    }

	void CreateCommandBuffers0() 
	{
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers0.size());

        if (vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, m_CommandBuffers0.data()) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate command buffers!");
    }

	void CreateCommandBuffers1() 
	{
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers1.size());

        if (vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, m_CommandBuffers1.data()) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate command buffers!");
    }

	void CreateGraphicsPipeline()
	{
		auto vertShaderCode = CompileShader(s_VertexShader, shaderc_glsl_vertex_shader);
        auto fragShaderCode = CompileShader(s_FragmentShader, shaderc_glsl_fragment_shader);

        VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = Vertex::GetBindingDescription();
        auto attributeDescriptions = Vertex::GetAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

       std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState = {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(m_LogicalDevice, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline layout!");

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.renderPass = m_Renderpass0;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(m_LogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
            throw std::runtime_error("Failed to create graphics pipeline!");

        vkDestroyShaderModule(m_LogicalDevice, fragShaderModule, nullptr);
        vkDestroyShaderModule(m_LogicalDevice, vertShaderModule, nullptr);
	}

	void CreateVertexBuffer()
	{
        VkDeviceSize bufferSize = sizeof(s_Vertices[0]) * s_Vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_LogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, s_Vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(m_LogicalDevice, stagingBufferMemory);

        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);
        CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

        vkDestroyBuffer(m_LogicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(m_LogicalDevice, stagingBufferMemory, nullptr);
    }

    void CreateIndexBuffer() 
	{
        VkDeviceSize bufferSize = sizeof(s_Indices[0]) * s_Indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_LogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, s_Indices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(m_LogicalDevice, stagingBufferMemory);

        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);
        CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

        vkDestroyBuffer(m_LogicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(m_LogicalDevice, stagingBufferMemory, nullptr);
    }

};

int main(int argc, char* argv[])
{
	(void)argc; (void)argv;

	Application app;
	app.Run();

	return 0;
}
