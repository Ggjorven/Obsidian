#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanImage.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{

    class Device;

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

        // Methods

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

    // TODO: Image Slice
    // TODO: Image subresource view

}