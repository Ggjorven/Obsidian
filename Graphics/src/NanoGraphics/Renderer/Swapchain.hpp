#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/SwapchainSpec.hpp"
#include "NanoGraphics/Renderer/CommandList.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanSwapchain.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{

    class Device;
    class Image;

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

        // Creation/Destruction methods // Note: Copy elision (RVO/NRVO) ensures object is constructed directly in the caller's stack frame.
        inline CommandListPool AllocateCommandListPool(const CommandListPoolSpecification& specs) { return CommandListPool(*this, specs); }
        inline void FreePool(CommandListPool& pool) { m_Swapchain.FreePool(pool); }

        // Methods
        inline void AcquireNextImage() { m_Swapchain.AcquireNextImage(); }
        inline void Present() { m_Swapchain.Present(); }

        // Getters
        inline const SwapchainSpecification& GetSpecification() const { return m_Swapchain.GetSpecification(); }

        inline uint32_t GetCurrentFrame() const { return m_Swapchain.GetCurrentFrame(); }
        inline Image& GetImage(uint8_t frame) { return m_Swapchain.GetImage(frame); }
        inline const Image& GetImage(uint8_t frame) const { return m_Swapchain.GetImage(frame); }

    private:
        // Constructor
        Swapchain(const Device& device, const SwapchainSpecification& specs)
            : m_Swapchain(device, specs) {}

    private:
        Type m_Swapchain;

        friend class Device;
    };

}