#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/SwapchainSpec.hpp"
#include "NanoGraphics/Renderer/CommandList.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanSwapchain.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{

    class Device;
    class Swapchain;

    ////////////////////////////////////////////////////////////////////////////////////
    // ExecutionRegion
    ////////////////////////////////////////////////////////////////////////////////////
    class ExecutionRegion
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanExecutionRegion>
        >;
    public:
        // Destructor
        ~ExecutionRegion() = default;

        // Creation/Destruction methods // Note: Copy elision (RVO/NRVO) ensures object is constructed directly in the caller's stack frame.
        inline CommandListPool AllocateCommandListPool(const CommandListPoolSpecification& specs) const { return CommandListPool(*this, specs); }
        inline void FreePool(CommandListPool& pool) { m_Region.FreePool(pool); }

    private:
        // Constructor
        ExecutionRegion(const Swapchain& swapchain)
            : m_Region(swapchain) {}

    private:
        Type m_Region;

        friend class Swapchain;
    };

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

        const ExecutionRegion& GetExecutionRegion() const { return m_Swapchain.GetExecutionRegion(); }

    private:
        // Constructor
        Swapchain(const Device& device, const SwapchainSpecification& specs)
            : m_Swapchain(device, specs) {}

    private:
        Type m_Swapchain;

        friend class Device;
    };

}