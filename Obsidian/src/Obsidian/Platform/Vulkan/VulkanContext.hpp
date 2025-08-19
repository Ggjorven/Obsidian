#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/API.hpp"
#include "Obsidian/Renderer/DeviceSpec.hpp"

#include "Obsidian/Platform/Vulkan/Vulkan.hpp"
#include "Obsidian/Platform/Vulkan/VulkanDevices.hpp"

#include <Nano/Nano.hpp>

#include <tuple>
#include <array>

namespace Obsidian::Internal
{

    class VulkanContext;

#if defined(OB_API_VULKAN)
    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanContext
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanContext
    {
    public:
        // Settings
        inline constexpr static const std::tuple<uint8_t, uint8_t> Version = { 1, 3 };
        inline constexpr static auto ValidationLayers = std::to_array<const char*>({
            "VK_LAYER_KHRONOS_validation",

            #if defined(OB_PLATFORM_MACOS)
            "VK_LAYER_KHRONOS_synchronization2"
            #endif
        });
        inline constexpr static auto DeviceExtensions = std::to_array<const char*>({
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,

            #if defined(OB_PLATFORM_MACOS)
            "VK_KHR_portability_subset",
            #endif

            "VK_KHR_synchronization2",
            "VK_KHR_copy_commands2"
        });
    public:
        // Constructors & Destructor
        VulkanContext(void* window, DeviceMessageCallback messageCallback, DeviceDestroyCallback destroyCallback, std::span<const char*> extensions);
        ~VulkanContext();

        // Internal methods
        void SetDebugName(void* object, VkObjectType type, const std::string& name) const;

        void Warn(const std::string& message) const;
        void Error(const std::string& message) const;

        void Destroy(DeviceDestroyFn destroyFn) const;

        // Internal Getters
        inline VulkanPhysicalDevice& GetVulkanPhysicalDevice() { return m_PhysicalDevice.Get(); }
        inline const VulkanPhysicalDevice& GetVulkanPhysicalDevice() const { return m_PhysicalDevice.Get(); }
        inline VulkanLogicalDevice& GetVulkanLogicalDevice() { return m_LogicalDevice.Get(); }
        inline const VulkanLogicalDevice& GetVulkanLogicalDevice() const { return m_LogicalDevice.Get(); }

        inline VkInstance GetVkInstance() const { return m_Instance; }
        inline VkDebugUtilsMessengerEXT GetVkDebugger() const { return m_DebugMessenger; }

    private:
        // Private methods
        void InitInstance();
        void InitDevices(void* window, std::span<const char*> extensions);

    private:
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

        Nano::Memory::DeferredConstruct<VulkanPhysicalDevice> m_PhysicalDevice = {};
        Nano::Memory::DeferredConstruct<VulkanLogicalDevice, true> m_LogicalDevice = {};

        DeviceDestroyCallback m_DestroyCallback = nullptr;
    };
#endif

}