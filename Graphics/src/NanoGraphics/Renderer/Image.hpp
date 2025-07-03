#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanImage.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Image.hpp"
#include "NanoGraphics/Platform/Dummy/DummyImage.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics::Internal
{
    class VulkanSwapchain;
    class Dx12Swapchain;
}

namespace Nano::Graphics
{

    class Device;

    ////////////////////////////////////////////////////////////////////////////////////
    // Image
    ////////////////////////////////////////////////////////////////////////////////////
    class Image
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanImage>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12Image>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyImage>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyImage>
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
#if defined(NG_API_VULKAN) || defined(NG_API_DX12)
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
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanStagingImage>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12StagingImage>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyStagingImage>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyStagingImage>
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
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanSampler>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12Sampler>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummySampler>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummySampler>
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