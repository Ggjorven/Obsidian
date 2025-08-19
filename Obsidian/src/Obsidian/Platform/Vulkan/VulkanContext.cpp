#include "obpch.h"
#include "VulkanContext.hpp"

#include "Obsidian/Core/Logging.hpp"
#include "Obsidian/Core/Information.hpp"
#include "Obsidian/Utils/Profiler.hpp"

#include "Obsidian/Renderer/Device.hpp"

#if defined(OB_PLATFORM_DESKTOP)
    #define GLFW_INCLUDE_VULKAN
    #include <GLFW/glfw3.h>
#endif

namespace
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Static callback/functions
    ////////////////////////////////////////////////////////////////////////////////////
    static Obsidian::DeviceMessageCallback s_MessageCallback = {};

    ////////////////////////////////////////////////////////////////////////////////////
    // Surface name
    ////////////////////////////////////////////////////////////////////////////////////
    #if defined(OB_PLATFORM_WINDOWS)
        #define VK_KHR_SURFACE_TYPE_NAME "VK_KHR_win32_surface"
    #elif defined(OB_PLATFORM_LINUX)
        #define VK_KHR_SURFACE_TYPE_NAME "VK_KHR_xcb_surface"
    #elif defined(OB_PLATFORM_MACOS)
        #define VK_KHR_SURFACE_TYPE_NAME "VK_EXT_metal_surface"
    #else
        #error Nano Graphics Vulkan: Unsupported platform...
    #endif

    ////////////////////////////////////////////////////////////////////////////////////
    // Function wrappers
    ////////////////////////////////////////////////////////////////////////////////////
    static void LoadFunctionPointers(VkInstance instance)
    {
        using namespace Obsidian::Internal::VkExtension;
        
        g_vkCreateDebugUtilsMessengerEXT = reinterpret_cast<decltype(g_vkCreateDebugUtilsMessengerEXT)>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        g_vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<decltype(g_vkDestroyDebugUtilsMessengerEXT)>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        g_vkSetDebugUtilsObjectNameEXT = reinterpret_cast<decltype(g_vkSetDebugUtilsObjectNameEXT)>(vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT"));

        g_vkQueueSubmit2KHR = reinterpret_cast<decltype(g_vkQueueSubmit2KHR)>(vkGetInstanceProcAddr(instance, "vkQueueSubmit2KHR"));
        g_vkCmdCopyBuffer2KHR = reinterpret_cast<decltype(g_vkCmdCopyBuffer2KHR)>(vkGetInstanceProcAddr(instance, "vkCmdCopyBuffer2KHR"));
        g_vkCmdCopyImage2KHR = reinterpret_cast<decltype(g_vkCmdCopyImage2KHR)>(vkGetInstanceProcAddr(instance, "vkCmdCopyImage2KHR"));
        g_vkCmdCopyBufferToImage2KHR = reinterpret_cast<decltype(g_vkCmdCopyBufferToImage2KHR)>(vkGetInstanceProcAddr(instance, "vkCmdCopyBufferToImage2KHR"));
        g_vkCmdPipelineBarrier2KHR = reinterpret_cast<decltype(g_vkCmdPipelineBarrier2KHR)>(vkGetInstanceProcAddr(instance, "vkCmdPipelineBarrier2KHR"));
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
            s_MessageCallback(Obsidian::DeviceMessageType::Error, pCallbackData->pMessage);
            return VK_TRUE;
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            s_MessageCallback(Obsidian::DeviceMessageType::Warn, pCallbackData->pMessage);
            return VK_TRUE;
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            s_MessageCallback(Obsidian::DeviceMessageType::Info, pCallbackData->pMessage);
            return VK_TRUE;
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            s_MessageCallback(Obsidian::DeviceMessageType::Trace, pCallbackData->pMessage);
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
        for (const char* layerName : Obsidian::Internal::VulkanContext::ValidationLayers)
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

namespace Obsidian::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Init & Destroy
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanContext::VulkanContext(void* window, DeviceMessageCallback messageCallback, DeviceDestroyCallback destroyCallback, std::span<const char*> extensions)
        : m_DestroyCallback(destroyCallback)
    {
        OB_ASSERT(window, "[VulkanContext] No window was attached.");
        OB_ASSERT(destroyCallback, "[VulkanContext] No destroy callback was passed in.");

        if constexpr (Information::Validation)
        {
            s_MessageCallback = messageCallback;

            if (static_cast<bool>(!messageCallback))
            {
                OB_LOG_WARN("[VulkanContext] Validation layers are enabled during Debug & Release builds, but no MessageCallback was passed in. Was this intentional?");
            }
        }

        InitInstance();
        InitDevices(window, extensions);
    }

    VulkanContext::~VulkanContext()
    {
        // Note: No need to 'destroy' the physical device since it was something we selected, not created.
        m_LogicalDevice.Destroy();

        if constexpr (Information::Validation)
        {
            if (m_DebugMessenger)
                VkExtension::g_vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
        }

        vkDestroyInstance(m_Instance, nullptr);
    }

    void VulkanContext::SetDebugName(void* object, VkObjectType type, const std::string& name) const
    {
        #if !defined(OB_CONFIG_DIST)
            VkDebugUtilsObjectNameInfoEXT nameInfo = {};
            nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
            nameInfo.objectType = type;
            nameInfo.objectHandle = reinterpret_cast<uint64_t>(object);
            nameInfo.pObjectName = name.c_str();

            VK_VERIFY(VkExtension::g_vkSetDebugUtilsObjectNameEXT(m_LogicalDevice->GetVkDevice(), &nameInfo));
        #else
            (void)object; (void)type; (void)name;
        #endif
    }

    void VulkanContext::Warn(const std::string& message) const
    {
        #if !defined(OB_CONFIG_DIST)
            if (static_cast<bool>(!s_MessageCallback)) [[unlikely]]
                return;

            s_MessageCallback(DeviceMessageType::Warn, message);
        #else
            (void)message;
        #endif
    }

    void VulkanContext::Error(const std::string& message) const
    {
        #if !defined(OB_CONFIG_DIST)
            if (static_cast<bool>(!s_MessageCallback)) [[unlikely]]
                return;

            s_MessageCallback(DeviceMessageType::Error, message);
        #else
            (void)message;
        #endif
    }

    void VulkanContext::Destroy(DeviceDestroyFn destroyFn) const
    {
        OB_ASSERT(m_DestroyCallback, "[VkContext] Destroy callback is invalid.");
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
        if constexpr (Information::Validation)
        {
            if (!validationSupport)
                OB_LOG_WARN("[VulkanContext] Requested validation layers, but no support found.");
        }

        std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_SURFACE_TYPE_NAME };
        if constexpr (Information::Validation)
        {
            if (validationSupport)
            {
                instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
                instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            }
        }

        #if defined(OB_PLATFORM_MACOS)
            instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        #endif

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        #if defined(OB_PLATFORM_MACOS)
            createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        #endif
        createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
        createInfo.ppEnabledExtensionNames = instanceExtensions.data();

        if constexpr (Information::Validation)
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

        if constexpr (Information::Validation)
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
        if constexpr (Information::Validation)
        {
            // Load extension function pointers
            LoadFunctionPointers(m_Instance);

            if (validationSupport)
            {
                VK_VERIFY(VkExtension::g_vkCreateDebugUtilsMessengerEXT(m_Instance, &debugCreateInfo, nullptr, &m_DebugMessenger));
            }
        }
    }

    void VulkanContext::InitDevices(void* window, std::span<const char*> extensions)
    {
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        #if defined(OB_PLATFORM_DESKTOP)
            VK_VERIFY(glfwCreateWindowSurface(m_Instance, static_cast<GLFWwindow*>(window), nullptr, &surface));
        #endif
        
        std::set<const char*> extensionSet(extensions.begin(), extensions.end());
        extensionSet.insert(DeviceExtensions.begin(), DeviceExtensions.end());
        std::vector<const char*> fullExtensions(extensionSet.begin(), extensionSet.end());

        m_PhysicalDevice.Construct(m_Instance, surface, std::span<const char*>(fullExtensions));
        m_LogicalDevice.Construct(m_PhysicalDevice, std::span<const char*>(fullExtensions));

        if constexpr (Information::Validation)
        {
            SetDebugName(m_Instance, VK_OBJECT_TYPE_INSTANCE, "Instance");
            SetDebugName(m_PhysicalDevice.Get().GetVkPhysicalDevice(), VK_OBJECT_TYPE_PHYSICAL_DEVICE, "PhysicalDevice");
            SetDebugName(m_LogicalDevice.Get().GetVkDevice(), VK_OBJECT_TYPE_DEVICE, "Device");

            const QueueFamilyIndices indices = m_PhysicalDevice.Get().GetQueueFamilyIndices();
            if (indices.SameQueue())
            {
                SetDebugName(m_LogicalDevice.Get().GetVkQueue(CommandQueue::Graphics), VK_OBJECT_TYPE_QUEUE, "Queue");
            }
            else
            {
                if (m_LogicalDevice.Get().GetVkQueue(CommandQueue::Graphics) == m_LogicalDevice.Get().GetVkQueue(CommandQueue::Compute))
                {
                    SetDebugName(m_LogicalDevice.Get().GetVkQueue(CommandQueue::Present), VK_OBJECT_TYPE_QUEUE, "Present Queue");
                    SetDebugName(m_LogicalDevice.Get().GetVkQueue(CommandQueue::Graphics), VK_OBJECT_TYPE_QUEUE, "Graphics/Compute Queue");
                }
                else if (m_LogicalDevice.Get().GetVkQueue(CommandQueue::Graphics) == m_LogicalDevice.Get().GetVkQueue(CommandQueue::Present))
                {
                    SetDebugName(m_LogicalDevice.Get().GetVkQueue(CommandQueue::Compute), VK_OBJECT_TYPE_QUEUE, "Compute Queue");
                    SetDebugName(m_LogicalDevice.Get().GetVkQueue(CommandQueue::Graphics), VK_OBJECT_TYPE_QUEUE, "Graphics/Present Queue");

                }
                else if (m_LogicalDevice.Get().GetVkQueue(CommandQueue::Compute) == m_LogicalDevice.Get().GetVkQueue(CommandQueue::Present))
                {
                    SetDebugName(m_LogicalDevice.Get().GetVkQueue(CommandQueue::Graphics), VK_OBJECT_TYPE_QUEUE, "Graphics Queue");
                    SetDebugName(m_LogicalDevice.Get().GetVkQueue(CommandQueue::Compute), VK_OBJECT_TYPE_QUEUE, "Compute/Present Queue");
                }
                else
                {
                    SetDebugName(m_LogicalDevice.Get().GetVkQueue(CommandQueue::Graphics), VK_OBJECT_TYPE_QUEUE, "Graphics Queue");
                    SetDebugName(m_LogicalDevice.Get().GetVkQueue(CommandQueue::Compute), VK_OBJECT_TYPE_QUEUE, "Compute Queue");
                    SetDebugName(m_LogicalDevice.Get().GetVkQueue(CommandQueue::Present), VK_OBJECT_TYPE_QUEUE, "Present Queue");
                }
            }
        }

        vkDestroySurfaceKHR(m_Instance, surface, nullptr);
    }

}