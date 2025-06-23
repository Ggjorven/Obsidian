#pragma once

#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include <cstdint>
#include <string>

namespace Nano::Graphics
{

    class Image;

    ////////////////////////////////////////////////////////////////////////////////////
    // FramebufferAttachment
    ////////////////////////////////////////////////////////////////////////////////////
    class FramebufferAttachment
    {
    public:
        Image* ImagePtr = nullptr;
        ImageSubresourceSpecification Subresources = ImageSubresourceSpecification(0, 1, 0, 1);
        
        bool IsReadOnly = false;

    public:
        // Setters
        inline constexpr FramebufferAttachment& SetImage(Image& image) { ImagePtr = &image; return *this; }
        inline constexpr FramebufferAttachment& SetSubresources(const ImageSubresourceSpecification& subresources) { Subresources = subresources; return *this; }
        inline constexpr FramebufferAttachment& SetArraySlice(ArraySlice index) { Subresources.BaseArraySlice = index; Subresources.NumArraySlices = 1; return *this; }
        inline constexpr FramebufferAttachment& SetArraySliceRange(ArraySlice index, ArraySlice count) { Subresources.BaseArraySlice = index; Subresources.NumArraySlices = count; return *this; }
        inline constexpr FramebufferAttachment& SetMipLevel(MipLevel level) { Subresources.BaseMipLevel = level; Subresources.NumMipLevels = 1; return *this; }
        inline constexpr FramebufferAttachment& SetIsReadOnly(bool enabled) { IsReadOnly = enabled; return *this; }

        // Methods
        inline constexpr bool IsValid() const { return (ImagePtr != nullptr); }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // FramebufferSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    class FramebufferSpecification // Note: We currently only support 1 colour attachment since that is most used, adding support for more is very simple though
    {
    public:
        FramebufferAttachment ColourAttachment = {};
        FramebufferAttachment DepthAttachment = {};

        std::string DebugName = {};

    public:
        // Setters
        inline constexpr FramebufferSpecification& SetColourAttachment(Image& image) { ColourAttachment.ImagePtr = &image; return *this; }
        inline constexpr FramebufferSpecification& SetColourAttachment(Image& image, const ImageSubresourceSpecification& specs) { ColourAttachment.ImagePtr = &image; ColourAttachment.Subresources = specs; return *this; }
        inline constexpr FramebufferSpecification& SetColourAttachment(const FramebufferAttachment& attachment) { ColourAttachment = attachment; return *this; }
        inline constexpr FramebufferSpecification& SetDepthAttachment(Image& image) { DepthAttachment.ImagePtr = &image; return *this; }
        inline constexpr FramebufferSpecification& SetDepthAttachment(Image& image, const ImageSubresourceSpecification& specs) { DepthAttachment.ImagePtr = &image; DepthAttachment.Subresources = specs; return *this; }
        inline constexpr FramebufferSpecification& SetDepthAttachment(const FramebufferAttachment& attachment) { DepthAttachment = attachment; return *this; }
        inline FramebufferSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }
    };

}