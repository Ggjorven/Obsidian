#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/API.hpp"
#include "Obsidian/Renderer/SwapchainSpec.hpp"
#include "Obsidian/Renderer/CommandList.hpp"

#include "Obsidian/Platform/Vulkan/VulkanSwapchain.hpp"
#include "Obsidian/Platform/Dx12/Dx12Swapchain.hpp"
#include "Obsidian/Platform/Dummy/DummySwapchain.hpp"

#include <Nano/Nano.hpp>

namespace Obsidian
{

    class Device;
    class Image;

    ////////////////////////////////////////////////////////////////////////////////////
    // Swapchain
    ////////////////////////////////////////////////////////////////////////////////////
    class Swapchain
    {
    public:
        using Type = Nano::Types::SelectorType<Information::RenderingAPI,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanSwapchain>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12Swapchain>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummySwapchain>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummySwapchain>
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

        inline void AcquireNextImage() { m_Impl->AcquireNextImage(); }
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