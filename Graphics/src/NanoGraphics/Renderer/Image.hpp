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

        // Setters
        inline void SetCurrentState(ResourceState state) { m_Image.SetCurrentState(state); }

        // Getters
        inline const ImageSpecification& GetSpecification() const { return m_Image.GetSpecification(); }

        inline ResourceState GetCurrentState() const { return m_Image.GetCurrentState(); }

    private:
        // Constructor 
        Image(const Device& device, const ImageSpecification& specs)
            : m_Image(device, specs) {}

    private:
        Type m_Image;

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