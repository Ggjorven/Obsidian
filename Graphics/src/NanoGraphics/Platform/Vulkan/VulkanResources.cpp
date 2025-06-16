#include "ngpch.h"
#include "VulkanResources.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/Image.hpp"

namespace Nano::Graphics::Internal
{

    static_assert(std::is_same_v<Image::Type, VulkanImage>, "Current Image::Type is not VulkanImage and Vulkan source code is being compiled.");

    ////////////////////////////////////////////////////////////////////////////////////
    // Resolving methods
    ////////////////////////////////////////////////////////////////////////////////////
    ImageSliceSpecification ResolveImageSlice(const ImageSliceSpecification& sliceSpec, const ImageSpecification& imageSpec)
    {
        ImageSliceSpecification ret(sliceSpec);

        NG_ASSERT((sliceSpec.ImageMipLevel < imageSpec.MipLevels), "[ImageSlicSpec] Slice miplevels are more than there are in the image.");

        // Note: We shift right because each mip makes it smaller
        if (sliceSpec.Width == ImageSliceSpecification::FullSize)
            ret.Width = std::max(imageSpec.Width >> sliceSpec.ImageMipLevel, 1u);

        // Note: We shift right because each mip makes it smaller
        if (sliceSpec.Height == ImageSliceSpecification::FullSize)
            ret.Height = std::max(imageSpec.Height >> sliceSpec.ImageMipLevel, 1u);

        if (sliceSpec.Depth == ImageSliceSpecification::FullSize)
        {
            if (imageSpec.Dimension == ImageDimension::Image3D)
                ret.Depth = std::max(imageSpec.Depth >> sliceSpec.ImageMipLevel, 1u);
            else
                ret.Depth = 1;
        }

        return ret;
    }

    ImageSubresourceSpecification ResolveImageSubresouce(const ImageSubresourceSpecification& subresourceSpec, const ImageSpecification& imageSpec, bool singleMip)
    {
        ImageSubresourceSpecification ret;
        ret.BaseMipLevel = subresourceSpec.BaseMipLevel;

        if (singleMip)
        {
            ret.NumMipLevels = 1;
        }
        else
        {
            int lastMipLevelPlusOne = std::min(subresourceSpec.BaseMipLevel + subresourceSpec.NumMipLevels, imageSpec.MipLevels);
            ret.NumMipLevels = MipLevel(std::max(0u, lastMipLevelPlusOne - subresourceSpec.BaseMipLevel));
        }

        switch (imageSpec.Dimension)
        {
        case ImageDimension::Image1DArray:
        case ImageDimension::Image2DArray:
        case ImageDimension::ImageCube:
        case ImageDimension::ImageCubeArray:
        case ImageDimension::Image2DMSArray: 
        {
            ret.BaseArraySlice = subresourceSpec.BaseArraySlice;
            int lastArraySlicePlusOne = std::min(subresourceSpec.BaseArraySlice + subresourceSpec.NumArraySlices, imageSpec.ArraySize);
            ret.NumArraySlices = ArraySlice(std::max(0u, lastArraySlicePlusOne - subresourceSpec.BaseArraySlice));
            break;
        }

        default:
            ret.BaseArraySlice = 0;
            ret.NumArraySlices = 1;
            break;
        }

        return ret;
    }

    BufferRange ResolveBufferRange(const BufferRange& range, const BufferSpecification& specs)
    {
        BufferRange ret(range);

        if (range.Size == BufferRange::FullSize)
            ret.Size = specs.Size;

        return ret;
    }

    uint32_t ResourceTypeToRegisterOffset(const VulkanBindingOffsets& bindingOffsets, ResourceType type)
    {
        switch (type)
        {
        case ResourceType::Image:
        case ResourceType::StorageBuffer:
        //case ResourceType::RayTracingAccelStruct:
            return bindingOffsets.ShaderResource;

        case ResourceType::ImageUnordered:
        case ResourceType::StorageBufferUnordered:
            return bindingOffsets.UnorderedAccess;

        case ResourceType::UniformBuffer:
        case ResourceType::PushConstants:
            return bindingOffsets.UniformBuffer;
            break;

        case ResourceType::Sampler:
            return bindingOffsets.Sampler;

        default:
            break;
        }

        NG_UNREACHABLE();
        return 0;
    }

}