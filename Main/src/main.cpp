#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <shaderc/shaderc.hpp>

#include <vulkan/vulkan.h>

#include <cstdint>
#include <iostream>
#include <array>

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
static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) 
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) 
        func(instance, debugMessenger, pAllocator);
}

// Application class

int main(int argc, char* argv[])
{
	std::cout << "Hello, world!" << std::endl;
	return 0;
}
