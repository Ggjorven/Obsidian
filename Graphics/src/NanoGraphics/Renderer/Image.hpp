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
    // StagingImage // TODO: Implement after implementing buffers
    ////////////////////////////////////////////////////////////////////////////////////
    class StagingImage : public Traits::NoCopy
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            //Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanStagingImage>
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, int> // TEMP
        >;
    public:
        // Destructor
        ~StagingImage() = default;

        // Getters
        //inline const StagingImageSpecification& GetSpecification() const { return m_Image.GetSpecification(); }

    private:
        // Constructor 
        //StagingImage(const Device& device, const StagingImageSpecification& specs)
        //    : m_Image(device, specs) {}

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