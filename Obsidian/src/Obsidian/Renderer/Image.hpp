#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/API.hpp"
#include "Obsidian/Renderer/ImageSpec.hpp"

#include "Obsidian/Platform/Vulkan/VulkanImage.hpp"
#include "Obsidian/Platform/Dx12/Dx12Image.hpp"
#include "Obsidian/Platform/Dummy/DummyImage.hpp"

#include <Nano/Nano.hpp>

namespace Obsidian::Internal
{
    class VulkanSwapchain;
    class Dx12Swapchain;
}

namespace Obsidian
{

    class Device;

    ////////////////////////////////////////////////////////////////////////////////////
    // Image
    ////////////////////////////////////////////////////////////////////////////////////
    class Image
    {
    public:
        using Type = Nano::Types::SelectorType<Information::RenderingAPI,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanImage>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12Image>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyImage>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyImage>
        >;
    public:
        // Destructor
        ~Image() = default;

        // Getters
        inline const ImageSpecification& GetSpecification() const { return m_Impl->GetSpecification(); }

    public: //private:
        // Constructor 
        inline Image(const Device& device, const ImageSpecification& specs) { m_Impl.Construct(device, specs); }

    private:
#if defined(OB_API_VULKAN) || defined(OB_API_DX12)
        // Private constructor for swapchains
        inline Image(const Device& device) { m_Impl.Construct(device); }
#endif

    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class Device;
        friend class APICaster;

        // Note: We have API friend classes since Swapchain images don't need to be created
        // They just need to be set inside of a class as a wrapper type.
        friend class Internal::VulkanSwapchain;
        friend class Internal::Dx12Swapchain;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // StagingImage
    ////////////////////////////////////////////////////////////////////////////////////
    class StagingImage
    {
    public:
        using Type = Nano::Types::SelectorType<Information::RenderingAPI,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanStagingImage>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12StagingImage>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyStagingImage>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyStagingImage>
        >;
    public:
        // Destructor
        ~StagingImage() = default;

        // Getters
        inline const ImageSpecification& GetSpecification() const { return m_Impl->GetSpecification(); }

    public: //private:
        // Constructor 
        inline StagingImage(const Device& device, const ImageSpecification& specs, CpuAccessMode cpuAccess) { m_Impl.Construct(device, specs, cpuAccess); }

    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class Device;
        friend class APICaster;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Sampler
    ////////////////////////////////////////////////////////////////////////////////////
    class Sampler
    {
    public:
        using Type = Nano::Types::SelectorType<Information::RenderingAPI,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanSampler>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12Sampler>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummySampler>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummySampler>
        >;
    public:
        // Destructor
        ~Sampler() = default;

        // Getters
        inline const SamplerSpecification& GetSpecification() const { return m_Impl->GetSpecification(); }

    public: //private:
        // Constructor 
        inline Sampler(const Device& device, const SamplerSpecification& specs) { m_Impl.Construct(device, specs); }

    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class Device;
        friend class APICaster;
    };

}