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

    ////////////////////////////////////////////////////////////////////////////////////
    // ImageSlice
    ////////////////////////////////////////////////////////////////////////////////////
    // TODO: Image Slice

    ////////////////////////////////////////////////////////////////////////////////////
    // ImageSubresourceView
    ////////////////////////////////////////////////////////////////////////////////////
    class ImageSubresourceView : public Traits::NoCopy
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanImageSubresourceView>
        >;
    public:
        // Destructor
        ~ImageSubresourceView() = default;

        // Methods

        // Getters
        inline const ImageSubresourceSpecification& GetSpecification() const { return m_ImageSubresourceView.GetSpecification(); }

    private:
        // Constructor 
        ImageSubresourceView(const Image& image, const ImageSubresourceSpecification& specs)
            : m_ImageSubresourceView(image, specs) {}

    private:
        Type m_ImageSubresourceView;

        friend class Image;
    };


}