#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanImage.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{

    class Device;
    class ImageSubresourceView;

    ////////////////////////////////////////////////////////////////////////////////////
    // Image
    ////////////////////////////////////////////////////////////////////////////////////
    class Image : public Traits::NoCopy
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanImage>
        >;
    public:
        // Destructor
        ~Image() = default;

        // Getters
        inline const ImageSpecification& GetSpecification() const { return m_Image.GetSpecification(); }

    private:
        // Constructor 
        Image(const Device& device, const ImageSpecification& specs)
            : m_Image(device, specs) {}

    private:
        Type m_Image;

        friend class Device;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // StagingImage
    ////////////////////////////////////////////////////////////////////////////////////
    class StagingImage : public Traits::NoCopy
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanStagingImage>
        >;
    public:
        // Destructor
        ~StagingImage() = default;

        // Getters
        inline const ImageSpecification& GetSpecification() const { return m_StagingImage.GetSpecification(); }

    private:
        // Constructor 
        StagingImage(const Device& device, const ImageSpecification& specs, CpuAccessMode cpuAccess)
            : m_StagingImage(device, specs, cpuAccess) {}

    private:
        Type m_StagingImage;

        friend class Device;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Sampler
    ////////////////////////////////////////////////////////////////////////////////////
    class Sampler : public Traits::NoCopy
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanSampler>
        >;
    public:
        // Destructor
        ~Sampler() = default;

        // Getters
        inline const SamplerSpecification& GetSpecification() const { return m_Sampler.GetSpecification(); }

    private:
        // Constructor 
        Sampler(const Device& device, const SamplerSpecification& specs)
            : m_Sampler(device, specs) {}

    private:
        Type m_Sampler;

        friend class Device;
    };


}