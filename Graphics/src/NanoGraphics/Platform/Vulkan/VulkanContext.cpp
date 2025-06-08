#include "ngpch.h"
#include "VulkanContext.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#if defined(NG_PLATFORM_DESKTOP)
    #define GLFW_INCLUDE_VULKAN
    #include <GLFW/glfw3.h>
#endif

namespace
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Static callback/functions
    ////////////////////////////////////////////////////////////////////////////////////
    static Nano::Graphics::DeviceMessageCallback s_MessageCallback = {};

    static PFN_vkCreateDebugUtilsMessengerEXT   s_vkCreateDebugUtilsMessengerEXT = nullptr;
    static PFN_vkDestroyDebugUtilsMessengerEXT  s_vkDestroyDebugUtilsMessengerEXT = nullptr;
    static PFN_vkSetDebugUtilsObjectNameEXT     s_vkSetDebugUtilsObjectNameEXT = nullptr;

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
    static void GetvkCreateDebugUtilsMessengerEXT(VkInstance instance)
    {
        if (!s_vkCreateDebugUtilsMessengerEXT)
        {
            s_vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
            NG_ASSERT(s_vkCreateDebugUtilsMessengerEXT, "Extension not present.");
        }
    }

    static void GetvkDestroyDebugUtilsMessengerEXT(VkInstance instance)
    {
        if (!s_vkDestroyDebugUtilsMessengerEXT)
        {
            s_vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
            NG_ASSERT(s_vkDestroyDebugUtilsMessengerEXT, "Extension not present.");
        }
    }

    static void GetvkSetDebugUtilsObjectNameEXT(VkInstance instance)
    {
        if (!s_vkSetDebugUtilsObjectNameEXT)
        {
            s_vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT"));
            NG_ASSERT(s_vkSetDebugUtilsObjectNameEXT, "Extension not present.");
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Support/Helper functions
    ////////////////////////////////////////////////////////////////////////////////////
    static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
    {
        if (static_cast<bool>(!s_MessageCallback)) [[unlikely]]
            return VK_FALSE;

        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            // Note for future: Make sure to check if the vkQueuePresentKHR is NOT waiting on the imageAvailable semaphore, as it will cause a deadlock and many errors.
            s_MessageCallback(Nano::Graphics::DeviceMessageType::Error, pCallbackData->pMessage);
            return VK_TRUE;
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            s_MessageCallback(Nano::Graphics::DeviceMessageType::Warn, pCallbackData->pMessage);
            return VK_TRUE;
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            s_MessageCallback(Nano::Graphics::DeviceMessageType::Info, pCallbackData->pMessage);
            return VK_TRUE;
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            s_MessageCallback(Nano::Graphics::DeviceMessageType::Trace, pCallbackData->pMessage);
            return VK_TRUE;
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
    VulkanContext::VulkanContext(void* window, DeviceMessageCallback messageCallback, DeviceDestroyCallback destroyCallback, std::span<const char*> extensions)
        : m_DestroyCallback(destroyCallback)
    {
        NG_ASSERT(window, "[VulkanContext] No window was attached.");
        NG_ASSERT(destroyCallback, "[VulkanContext] No destroy callback was passed in.");

        if constexpr (Validation)
        {
            s_MessageCallback = messageCallback;

            if (static_cast<bool>(!messageCallback))
            {
                NG_LOG_WARN("[VulkanContext] Validation layers are enabled during Debug & Release builds, but no MessageCallback was passed in. Was this intentional?");
            }
        }

        InitInstance();
        InitDevices(window, extensions);
    }

    VulkanContext::~VulkanContext()
    {
        // Note: No need to 'destroy' the physical device since it was something we selected, not created.
        m_LogicalDevice.Destroy();

        if constexpr (Validation)
        {
            if (m_DebugMessenger)
                s_vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        }

        vkDestroyInstance(m_Instance, nullptr);
    }

    void VulkanContext::SetDebugName(void* object, VkObjectType type, const std::string& name) const
    {
        #if !defined(NG_CONFIG_DIST)
            VkDebugUtilsObjectNameInfoEXT nameInfo = {};
            nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            nameInfo.objectType = type;
            nameInfo.objectHandle = reinterpret_cast<uint64_t>(object);
            nameInfo.pObjectName = name.c_str();

            s_vkSetDebugUtilsObjectNameEXT(m_LogicalDevice->GetVkDevice(), &nameInfo);
        #endif
    }

    void VulkanContext::Warn(const std::string& message) const
    {
        #if !defined(NG_CONFIG_DIST)
            if (static_cast<bool>(!s_MessageCallback)) [[unlikely]]
                return;

            s_MessageCallback(DeviceMessageType::Warn, message);
        #endif
    }

    void VulkanContext::Error(const std::string& message) const
    {
        #if !defined(NG_CONFIG_DIST)
            if (static_cast<bool>(!s_MessageCallback)) [[unlikely]]
                return;

            s_MessageCallback(DeviceMessageType::Error, message);
        #endif
    }

    void VulkanContext::Destroy(DeviceDestroyFn destroyFn) const
    {
        NG_ASSERT(m_DestroyCallback, "[VkContext] Destroy callback is invalid.");
        m_DestroyCallback(destroyFn);
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
            // Load extension function pointers
            GetvkCreateDebugUtilsMessengerEXT(m_Instance);
            GetvkDestroyDebugUtilsMessengerEXT(m_Instance);
            GetvkSetDebugUtilsObjectNameEXT(m_Instance);

            if (validationSupport)
            {
                s_vkCreateDebugUtilsMessengerEXT(m_Instance, &debugCreateInfo, nullptr, &m_DebugMessenger);
            }
        }
    }

    void VulkanContext::InitDevices(void* window, std::span<const char*> extensions)
    {
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        #if defined(NG_PLATFORM_DESKTOP)
            VK_VERIFY(glfwCreateWindowSurface(m_Instance, static_cast<GLFWwindow*>(window), nullptr, &surface));
        #endif
        
        std::set<const char*> extensionSet(extensions.begin(), extensions.end());
        extensionSet.insert(DeviceExtensions.begin(), DeviceExtensions.end());
        std::vector<const char*> fullExtensions(extensionSet.begin(), extensionSet.end());

        m_PhysicalDevice.Construct(m_Instance, surface, std::span<const char*>(fullExtensions));
        m_LogicalDevice.Construct(m_PhysicalDevice, std::span<const char*>(fullExtensions));
        m_Queues.Construct(*this);

        vkDestroySurfaceKHR(m_Instance, surface, nullptr);
    }

}