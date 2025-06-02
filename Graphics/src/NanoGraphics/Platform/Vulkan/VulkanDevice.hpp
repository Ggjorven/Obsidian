#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/DeviceSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanDevices.hpp"

#include <Nano/Nano.hpp>

#include <tuple>
#include <array>

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanDevice
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanDevice : public Traits::NoMove
    {
    public:
        // Settings
        inline constexpr static const std::tuple<uint8_t, uint8_t> Version = { 1, 3 };
        inline constexpr static const bool Validation = (Information::Configuration != Information::Structs::Configuration::Dist);
        inline constexpr static auto ValidationLayers = std::to_array<const char*>({ 
            "VK_LAYER_KHRONOS_validation",

            #if defined(NG_PLATFORM_MACOS)
            "VK_LAYER_KHRONOS_synchronization2" 
            #endif
        });
        inline constexpr static auto DeviceExtensions = std::to_array<const char*>({
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,

            #if defined(NG_PLATFORM_MACOS)
            "VK_KHR_portability_subset"
            #endif
        });
    public:
        // Constructors & Destructor
        VulkanDevice(const DeviceSpecification& specs);
        ~VulkanDevice();

        // Internal Getters
        VulkanLogicalDevice& GetVulkanLogicalDevice() { return m_LogicalDevice.Get(); }
        VulkanPhysicalDevice& GetVulkanPhysicalDevice() { return m_PhysicalDevice.Get(); }

        inline VkInstance GetVkInstance() const { return m_Instance; }
        inline VkDebugUtilsMessengerEXT GetVkDebugger() const { return m_DebugMessenger; }

    private:
        // Private methods
        void InitInstance();
        void InitDevices(void* window, std::span<const char*> extensions);

    private:
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;

        Memory::DeferredConstruct<VulkanLogicalDevice, true> m_LogicalDevice = {};
        Memory::DeferredConstruct<VulkanPhysicalDevice> m_PhysicalDevice = {};
    };

}