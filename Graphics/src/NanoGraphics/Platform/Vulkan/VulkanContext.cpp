#include "ngpch.h"
#include "VulkanContext.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/GraphicsContext.hpp"

#if defined(NG_PLATFORM_DESKTOP)
    #define GLFW_INCLUDE_VULKAN
    #include <GLFW/glfw3.h>
#endif

namespace
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Surface name
    ////////////////////////////////////////////////////////////////////////////////////
    #if defined(NG_PLATFORM_WINDOWS)
        #define VK_KHR_SURFACE_TYPE_NAME "VK_KHR_win32_surface"
    #elif defined(NG_PLATFORM_LINUX)
        #define VK_KHR_SURFACE_TYPE_NAME "VK_KHR_xcb_surface"
    #elif defined(NG_PLATFORM_MACOS)
        #define VK_KHR_SURFACE_TYPE_NAME "VK_EXT_metal_surface"
    #else
        #error Nano Graphics Vulkan: Unsupported platform...
    #endif

    ////////////////////////////////////////////////////////////////////////////////////
    // Function wrappers
    ////////////////////////////////////////////////////////////////////////////////////
    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

        if (func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);

        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

        if (func != nullptr)
            func(instance, debugMessenger, pAllocator);
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            // Note for future: Make sure to check if the vkQueuePresentKHR is NOT waiting on the imageAvailable semaphore, as it will cause a deadlock and many errors.
            NG_LOG_ERROR("Validation Error: {0}", pCallbackData->pMessage);
            return VK_TRUE;
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            NG_LOG_WARN("Validation Warning: {0}", pCallbackData->pMessage);
            return VK_FALSE;
        }

        return VK_FALSE;
    }

    static bool ValidationLayersSupported()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        // Check if all requested layers are actually accessible
        for (const char* layerName : Nano::Graphics::Internal::VulkanContext::ValidationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
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

}

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Init & Destroy
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanContext::Init(void* window)
    {
        NG_ASSERT(window, "[VulkanContext] No window was attached.");

        InitInstance();
        InitDevices(window);

        VulkanAllocator::Init();
    }

    void VulkanContext::Destroy()
    {
        VulkanAllocator::Destroy();

        // Note: No need to 'destroy' the physical device since it was something we selected, not created.
        m_Device.Destroy();

        if constexpr (Validation)
        {
            if (m_DebugMessenger)
                DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        }

        vkDestroyInstance(m_Instance, nullptr);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Static Getters
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanDevice& VulkanContext::GetVulkanDevice()
    {
        return GraphicsContext::GetInternalContext().m_Device;
    }

    VulkanPhysicalDevice& VulkanContext::GetVulkanPhysicalDevice()
    {
        return GraphicsContext::GetInternalContext().m_PhysicalDevice;
    }

    VkInstance VulkanContext::GetVkInstance()
    {
        return GraphicsContext::GetInternalContext().m_Instance;
    }

    VkDebugUtilsMessengerEXT VulkanContext::GetVkDebugger()
    {
        return GraphicsContext::GetInternalContext().m_DebugMessenger;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanContext::InitInstance()
    {
        ///////////////////////////////////////////////////////////
        // Instance Creation
        ///////////////////////////////////////////////////////////
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Nano Application";
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0, std::get<0>(Version), std::get<1>(Version), 0);
        appInfo.pEngineName = "Nano Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0, std::get<0>(Version), std::get<1>(Version), 0);
        appInfo.apiVersion = VK_MAKE_API_VERSION(0, std::get<0>(Version), std::get<1>(Version), 0);

        // Check for validation layer support
        bool validationSupport = ValidationLayersSupported();
        if constexpr (Validation)
        {
            if (!validationSupport)
                NG_LOG_WARN("[VulkanContext] Requested validation layers, but no support found.");
        }

        std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_SURFACE_TYPE_NAME };
        if constexpr (Validation)
        {
            if (validationSupport)
            {
                instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
                instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            }
        }

        #if defined(NG_PLATFORM_MACOS)
            instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        #endif

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        #if defined(NG_PLATFORM_MACOS)
            createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        #endif
        createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
        createInfo.ppEnabledExtensionNames = instanceExtensions.data();

        if constexpr (Validation)
        {
            if (validationSupport)
            {
                createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
                createInfo.ppEnabledLayerNames = ValidationLayers.data();
            }
            else
            {
                createInfo.enabledLayerCount = 0;
            }
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        // Note: Setup the debug messenger also for the create instance
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = &VulkanDebugCallback;

        if constexpr (Validation)
        {
            if (validationSupport)
            {
                createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
                createInfo.ppEnabledLayerNames = ValidationLayers.data();

                createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
            }
            else
            {
                createInfo.enabledLayerCount = 0;
                createInfo.pNext = nullptr;
            }
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        VK_VERIFY(vkCreateInstance(&createInfo, nullptr, &m_Instance));

        ///////////////////////////////////////////////////////////
        // Debugger Creation
        ///////////////////////////////////////////////////////////
        if constexpr (Validation)
        {
            if (validationSupport)
            {
                VK_VERIFY(CreateDebugUtilsMessengerEXT(m_Instance, &debugCreateInfo, nullptr, &m_DebugMessenger));
            }
        }
    }

    void VulkanContext::InitDevices(void* window)
    {
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        #if defined(NG_PLATFORM_DESKTOP)
            VK_VERIFY(glfwCreateWindowSurface(m_Instance, static_cast<GLFWwindow*>(window), nullptr, &surface));
        #endif

        m_PhysicalDevice.Construct(surface);
        m_Device.Construct(surface, m_PhysicalDevice);

        vkDestroySurfaceKHR(m_Instance, surface, nullptr);
    }

}