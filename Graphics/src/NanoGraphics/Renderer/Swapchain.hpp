#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/SwapchainSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanSwapchain.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{

    class Device;

    ////////////////////////////////////////////////////////////////////////////////////
    // Swapchain
    ////////////////////////////////////////////////////////////////////////////////////
    class Swapchain
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanSwapchain>
        >;
    public:
        // Destructor
        ~Swapchain() = default;

        // Methods
        inline void AcquireNextImage() { m_Swapchain.AcquireNextImage(); }
        inline void Present() { m_Swapchain.Present(); }

        // Getters
        inline const SwapchainSpecification& GetSpecification() const { return m_Swapchain.GetSpecification(); }

    private:
        // Constructor
        Swapchain(const Device& device, const SwapchainSpecification& specs)
            : m_Swapchain(device, specs) {}

    private:
        Type m_Swapchain;

        friend class Device;
    };

}