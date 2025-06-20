#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/SwapchainSpec.hpp"
#include "NanoGraphics/Renderer/CommandList.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanSwapchain.hpp"
#include "NanoGraphics/Platform/Dummy/DummySwapchain.hpp"

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
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanSwapchain>,
            Types::EnumToType<Information::Structs::RenderingAPI::D3D12, Internal::DummySwapchain>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummySwapchain>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummySwapchain>
        >;
    public:
        // Destructor
        ~Swapchain() = default;

        // Creation/Destruction methods // Note: Copy elision (RVO/NRVO) ensures object is constructed directly in the caller's stack frame.
        inline CommandListPool AllocateCommandListPool(const CommandListPoolSpecification& specs) { return CommandListPool(*this, specs); }
        inline void FreePool(CommandListPool& pool) { m_Swapchain->FreePool(pool); }

        // Methods
        inline void Resize(uint32_t width, uint32_t height) { m_Swapchain->Resize(width, height); }
        inline void Resize(uint32_t width, uint32_t height, bool vsync, Format colourFormat, ColourSpace colourSpace) { m_Swapchain->Resize(width, height, vsync, colourFormat, colourSpace); }

        inline void AcquireNextImage() { m_Swapchain->AcquireNextImage(); }
        inline void Present() { m_Swapchain->Present(); }

        // Getters
        inline const SwapchainSpecification& GetSpecification() const { return m_Swapchain->GetSpecification(); }

        inline uint32_t GetCurrentFrame() const { return m_Swapchain->GetCurrentFrame(); }
        inline Image& GetImage(uint8_t frame) { return m_Swapchain->GetImage(frame); }
        inline const Image& GetImage(uint8_t frame) const { return m_Swapchain->GetImage(frame); }

    public: //private:
        // Constructor
        inline Swapchain(const Device& device, const SwapchainSpecification& specs) { m_Swapchain.Construct(device, specs); }

    private:
        // Helper getter
        inline Type& APICasterGet() { return m_Swapchain.Get(); }

    private:
        Internal::APIObject<Type> m_Swapchain = {};

        friend class Device;
        friend class APICaster;
    };

}