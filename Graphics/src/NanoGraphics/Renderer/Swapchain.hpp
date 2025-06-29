#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/SwapchainSpec.hpp"
#include "NanoGraphics/Renderer/CommandList.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanSwapchain.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Swapchain.hpp"
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
            Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12Swapchain>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummySwapchain>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummySwapchain>
        >;
    public:
        // Destructor
        ~Swapchain() = default;

        // Creation/Destruction methods // Note: Copy elision (RVO/NRVO) ensures object is constructed directly in the caller's stack frame.
        inline CommandListPool AllocateCommandListPool(const CommandListPoolSpecification& specs) { return CommandListPool(*this, specs); }
        inline void FreePool(CommandListPool& pool) { m_Impl->FreePool(pool); }

        // Methods
        inline void Resize(uint32_t width, uint32_t height) { m_Impl->Resize(width, height); }
        inline void Resize(uint32_t width, uint32_t height, bool vsync, Format colourFormat, ColourSpace colourSpace) { m_Impl->Resize(width, height, vsync, colourFormat, colourSpace); }

        inline void AcquireNextImage() { m_Impl->AcquireNextImage(); } // Note: An acquired image will be ResourceState::RenderTarget
        inline void Present() { m_Impl->Present(); } // Note: The image must be in present (often done through renderpass EndState)

        // Getters
        inline const SwapchainSpecification& GetSpecification() const { return m_Impl->GetSpecification(); }

        inline uint8_t GetCurrentFrame() const { return m_Impl->GetCurrentFrame(); }
        inline uint8_t GetAcquiredImage() const { return m_Impl->GetAcquiredImage(); }

        inline Image& GetImage(uint8_t frame) { return m_Impl->GetImage(frame); }
        inline const Image& GetImage(uint8_t frame) const { return m_Impl->GetImage(frame); }

        inline uint8_t GetImageCount() const { return m_Impl->GetImageCount(); }

    public: //private:
        // Constructor
        inline Swapchain(const Device& device, const SwapchainSpecification& specs) { m_Impl.Construct(device, specs); }

    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class Device;
        friend class APICaster;
    };

}