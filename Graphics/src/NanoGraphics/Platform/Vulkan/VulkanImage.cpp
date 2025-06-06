#include "ngpch.h"
#include "VulkanImage.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/Image.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanResource.hpp"

namespace Nano::Graphics::Internal
{

    static_assert(std::is_same_v<Device::Type, VulkanDevice>, "Current Device::Type is not VulkanDevice and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<Image::Type, VulkanImage>, "Current Image::Type is not VulkanImage and Vulkan source code is being compiled.");
    static_assert(std::is_same_v<Sampler::Type, VulkanSampler>, "Current Sampler::Type is not VulkanSampler and Vulkan source code is being compiled.");

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanImageSubresourceView::VulkanImageSubresourceView(const Image& image, const ImageSubresourceSpecification& specs)
        : m_Image(*reinterpret_cast<const VulkanImage*>(&image)), m_Specification(specs)
    {
    }

    VulkanImageSubresourceView::~VulkanImageSubresourceView()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanImage::VulkanImage(const Device& device, const ImageSpecification& specs)
        : m_Device(*reinterpret_cast<const VulkanDevice*>(&device)), m_Specification(specs)
    {
        // Validation checks
        if constexpr (VulkanContext::Validation)
        {
            if (m_Specification.KeepResourceState && m_Specification.State == ResourceState::Unknown)
            {
                m_Device.GetContext().Error("[VulkanImage] KeepResourceState = true, but ResourceState is set to Unknowm which is not compatible. Disable KeepResourceState.");
            }
            if (!((specs.SampleCount >= 1) && (specs.SampleCount <= 64) && (specs.SampleCount & (specs.SampleCount - 1)) == 0))
            {
                m_Device.GetContext().Error(std::format("[VulkanImage] Invalid samplecount passed in: {0}", specs.SampleCount));
            }
        }

        // Creation
        m_Allocation = m_Device.GetAllocator().CreateImage(VMA_MEMORY_USAGE_AUTO, m_Image, 
            ImageDimensionToVkImageType(m_Specification.Dimension), 
            m_Specification.Width, m_Specification.Height, m_Specification.Depth, 
            m_Specification.MipLevels, m_Specification.ArraySize, 
            FormatToVkFormat(m_Specification.ImageFormat), VK_IMAGE_TILING_OPTIMAL, 
            ImageSpecificationToVkImageUsageFlags(m_Specification),
            SampleCountToVkSampleCountFlags(m_Specification.SampleCount)
        );
    }

    VulkanImage::VulkanImage(const Device& device, const ImageSpecification& specs, VkImage image)
        : m_Device(*reinterpret_cast<const VulkanDevice*>(&device)), m_Specification(specs), m_Image(image)
    {
    }

    VulkanImage::~VulkanImage()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    const VulkanImageSubresourceView& VulkanImage::GetSubresourceView(const ImageSubresourceSpecification& specs, ImageDimension dimension, Format format, VkImageUsageFlags usage, ImageSubresourceViewType viewType)
    {
        // Automatically set the dimension and format if not specified
        if (dimension == ImageDimension::Unknown)
            dimension = m_Specification.Dimension;
        if (format == Format::Unknown)
            format = m_Specification.ImageFormat;

        // Find the view in map
        auto cachekey = std::make_tuple(specs, viewType, dimension, format, usage);
        auto it = m_ImageViews.find(cachekey);
        if (it != m_ImageViews.end())
            return it->second;

        // Create new
        auto viewPair = m_ImageViews.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(cachekey), 
            std::forward_as_tuple(*reinterpret_cast<Image*>(this), specs)
        );
        auto& imageView = std::get<0>(viewPair)->second;

        VkFormat vkFormat = FormatToVkFormat(format);
        VkImageAspectFlags aspectflags = GuessSubresourceImageAspectFlags(vkFormat, viewType);
        VkImageViewType imageViewType = ImageDimensionToVkImageViewType(dimension);

        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_Image;
        createInfo.viewType = imageViewType;
        createInfo.format = vkFormat;
        createInfo.components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .a = VK_COMPONENT_SWIZZLE_IDENTITY
        };
        
        createInfo.subresourceRange.aspectMask = aspectflags;
        createInfo.subresourceRange.baseMipLevel = specs.BaseMipLevel;
        createInfo.subresourceRange.levelCount = specs.NumMipLevels;
        createInfo.subresourceRange.baseArrayLayer = specs.BaseArraySlice;
        createInfo.subresourceRange.layerCount = specs.NumArraySlices;

        VkImageViewUsageCreateInfo usageInfo = {};
        usageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO;
        usageInfo.usage = usage;

        if (usage != (VkImageUsageFlags)0)
            createInfo.pNext = &usageInfo;

        if (viewType == ImageSubresourceViewType::StencilOnly)
        {
            // D3D / HLSL puts stencil values in the second component to keep the illusion of combined depth/stencil.
            // Set a component swizzle so we appear to do the same.
            createInfo.components.g = VK_COMPONENT_SWIZZLE_R;
        }

        VK_VERIFY(vkCreateImageView(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &createInfo, m_Device.GetAllocator().GetCallbacks(), &imageView.m_ImageView));

        if (!m_Specification.DebugName.empty())
        {
            std::string debugName = std::string("ImageView for: ") + m_Specification.DebugName;
            m_Device.GetContext().SetDebugName(imageView.m_ImageView, VK_OBJECT_TYPE_IMAGE_VIEW, debugName.c_str());
        }

        return imageView;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanSampler::VulkanSampler(const Device& device, const SamplerSpecification& specs)
        : m_Device(*reinterpret_cast<const VulkanDevice*>(&device)), m_Specification(specs)
    {
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = FilterModeToVkFilter(m_Specification.MagFilter);
        samplerInfo.minFilter = FilterModeToVkFilter(m_Specification.MinFilter);
        samplerInfo.addressModeU = SamplerAddressModeToVkSamplerAddressMode(m_Specification.AddressU);
        samplerInfo.addressModeV = SamplerAddressModeToVkSamplerAddressMode(m_Specification.AddressV);
        samplerInfo.addressModeW = SamplerAddressModeToVkSamplerAddressMode(m_Specification.AddressW);

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(m_Device.GetContext().GetVulkanPhysicalDevice().GetVkPhysicalDevice(), &properties);

        samplerInfo.anisotropyEnable = ((m_Specification.MaxAnisotropy == SamplerSpecification::DisableMaxAnisotropyValue) ? VK_FALSE : VK_TRUE);
        samplerInfo.maxAnisotropy = ((m_Specification.MaxAnisotropy == SamplerSpecification::MaxMaxAnisotropyValue) ? properties.limits.maxSamplerAnisotropy : m_Specification.MaxAnisotropy); 

        samplerInfo.borderColor = Vec4ToBorderColor(specs.BorderColour);
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = ((m_Specification.ReductionType == SamplerReductionType::Comparison) ? VK_TRUE : VK_FALSE);
        samplerInfo.compareOp = VK_COMPARE_OP_LESS;

        samplerInfo.mipmapMode = ((FilterModeToVkFilter(m_Specification.MipFilter) == VK_FILTER_NEAREST) ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR);
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = std::numeric_limits<float>::max();
        samplerInfo.mipLodBias = m_Specification.MipBias;

        VK_VERIFY(vkCreateSampler(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &samplerInfo, m_Device.GetAllocator().GetCallbacks(), &m_Sampler));
    }

    VulkanSampler::~VulkanSampler()
    {
    }

}