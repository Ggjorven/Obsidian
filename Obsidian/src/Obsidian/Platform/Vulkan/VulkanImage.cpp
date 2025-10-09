#include "obpch.h"
#include "VulkanImage.hpp"

#include "Obsidian/Core/Logging.hpp"
#include "Obsidian/Utils/Profiler.hpp"

#include "Obsidian/Renderer/Device.hpp"
#include "Obsidian/Renderer/Image.hpp"

#include "Obsidian/Platform/Vulkan/VulkanDevice.hpp"

namespace Obsidian::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructors & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanImageSubresourceView::VulkanImageSubresourceView(const Image& image, const ImageSubresourceSpecification& specs)
        : m_Image(*api_cast<const VulkanImage*>(&image)), m_Specification(specs)
    {
    }

    VulkanImageSubresourceView::~VulkanImageSubresourceView()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanImage::VulkanImage(const Device& device)
        : m_Device(*api_cast<const VulkanDevice*>(&device)), m_Specification({})
    {
    }

    VulkanImage::VulkanImage(const Device& device, const ImageSpecification& specs)
        : m_Device(*api_cast<const VulkanDevice*>(&device)), m_Specification(specs)
    {
        OB_ASSERT(((m_Specification.Width != 0) && (m_Specification.Height != 0)), "[VkImage] Invalid width/height passed in.");
        OB_ASSERT((m_Specification.ImageFormat != Format::Unknown), "[VkImage] Invalid format passed in.");

        // Validation checks
        if constexpr (Information::Validation)
        {
            if (!((m_Specification.SampleCount >= 1) && (m_Specification.SampleCount <= 64) && (m_Specification.SampleCount & (m_Specification.SampleCount - 1)) == 0))
            {
                m_Device.GetContext().Error(std::format("[VkImage] Invalid samplecount passed in: {0}", m_Specification.SampleCount));
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

        if constexpr (Information::Validation)
        {
            if (!m_Specification.DebugName.empty())
            {
                m_Device.GetContext().SetDebugName(m_Image, VK_OBJECT_TYPE_IMAGE, std::string(m_Specification.DebugName));
                m_Device.GetContext().SetDebugName(m_Device.GetAllocator().GetUnderlyingMemory(m_Allocation), VK_OBJECT_TYPE_DEVICE_MEMORY, std::format("Memory for: {0}", m_Specification.DebugName));
            }
        }
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
            std::forward_as_tuple(*api_cast<Image*>(this), specs)
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
            /*.r*/ VK_COMPONENT_SWIZZLE_IDENTITY,
            /*.g*/ VK_COMPONENT_SWIZZLE_IDENTITY,
            /*.b*/ VK_COMPONENT_SWIZZLE_IDENTITY,
            /*.a*/ VK_COMPONENT_SWIZZLE_IDENTITY
        };
        
        createInfo.subresourceRange.aspectMask = aspectflags;
        createInfo.subresourceRange.baseMipLevel = specs.BaseMipLevel;
        createInfo.subresourceRange.levelCount = specs.NumMipLevels;
        createInfo.subresourceRange.baseArrayLayer = specs.BaseArraySlice;
        createInfo.subresourceRange.layerCount = specs.NumArraySlices;

        VkImageViewUsageCreateInfo usageInfo = {};
        usageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO;
        usageInfo.usage = usage;

        if (usage != static_cast<VkImageUsageFlags>(0))
            createInfo.pNext = &usageInfo;

        if (viewType == ImageSubresourceViewType::StencilOnly)
        {
            // D3D / HLSL puts stencil values in the second component to keep the illusion of combined depth/stencil.
            // Set a component swizzle so we appear to do the same.
            createInfo.components.g = VK_COMPONENT_SWIZZLE_R;
        }

        VK_VERIFY(vkCreateImageView(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &createInfo, VulkanAllocator::GetCallbacks(), &imageView.m_ImageView));

        if constexpr (Information::Validation)
        {
            if (!m_Specification.DebugName.empty())
                m_Device.GetContext().SetDebugName(imageView.m_ImageView, VK_OBJECT_TYPE_IMAGE_VIEW, std::format("ImageView for: {0}", m_Specification.DebugName));
        }

        return imageView;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Internal methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanImage::SetInternalData(const ImageSpecification& specs, VkImage image)
    {
        m_Specification = specs;
        m_Image = image;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanStagingImage::VulkanStagingImage(const Device& device, const ImageSpecification& specs, CpuAccessMode cpuAccessMode)
        : m_Device(*api_cast<const VulkanDevice*>(&device)), m_Specification(specs), m_SliceRegions(GetSliceRegions()), m_Buffer(device, BufferSpecification()
            .SetSize(GetBufferSize())
            .SetCPUAccess(cpuAccessMode)
            .SetPermanentState(specs.PermanentState)
            .SetDebugName(specs.DebugName)
        ) // Note: The buffer is automatically a TransferSrc (& TransferDst), we don't need to set anything special
    {
    }

    VulkanStagingImage::~VulkanStagingImage()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////////////////////////////////////////////////
    std::vector<VulkanStagingImage::Region> VulkanStagingImage::GetSliceRegions() const
    {
        std::vector<Region> regions;
        size_t currentOffset = 0;

        for (uint32_t mip = 0; mip < m_Specification.MipLevels; mip++)
        {
            auto sliceSize = ComputeSliceSize(mip);

            uint32_t depth = std::max(m_Specification.Depth >> mip, 1u);
            uint32_t numSlices = m_Specification.ArraySize * depth;

            for (uint32_t slice = 0; slice < numSlices; slice++)
            {
                regions.emplace_back(currentOffset, sliceSize);
                currentOffset = Nano::Memory::AlignOffset(currentOffset + sliceSize, 4);
            }
        }

        return regions;
    }

    size_t VulkanStagingImage::ComputeSliceSize(uint32_t mipLevel) const
    {
        const FormatInfo& formatInfo = FormatToFormatInfo(m_Specification.ImageFormat);

        uint32_t wInBlocks = std::max(((m_Specification.Width >> mipLevel) + formatInfo.BlockSize - 1) / formatInfo.BlockSize, 1u);
        uint32_t hInBlocks = std::max(((m_Specification.Height >> mipLevel) + formatInfo.BlockSize - 1) / formatInfo.BlockSize, 1u);

        size_t blockPitchBytes = static_cast<size_t>(wInBlocks) * formatInfo.BytesPerBlock;
        return blockPitchBytes * hInBlocks;
    }

    size_t VulkanStagingImage::GetBufferSize() const
    {
        OB_ASSERT(m_SliceRegions.size(), "[VkStagingImage] No slice regions created.");
        size_t size = m_SliceRegions.back().Offset + m_SliceRegions.back().Size;

        OB_ASSERT(size > 0, "[VkStagingImage] BufferSize is smaller or equal to 0.");
        return size;
    }

    VulkanStagingImage::Region VulkanStagingImage::GetSliceRegion(MipLevel mipLevel, ArraySlice arraySlice, uint32_t z) const
    {
        if (m_Specification.Depth != 1)
        {
            // Hard case, since each mip level has half the slices as the previous one.
            OB_ASSERT(arraySlice == 0, "[VkStagingImage] Must be first array slice if depth != 1.");
            OB_ASSERT(z < m_Specification.Depth, "[VkStagingImage] Depth parameter must be less than the image depth.");

            uint32_t mipDepth = m_Specification.Depth;
            uint32_t index = 0;
            while (mipLevel-- > 0)
            {
                index += mipDepth;
                mipDepth = std::max(mipDepth, uint32_t(1));
            }

            return m_SliceRegions[static_cast<size_t>(index) + z];
        }
        else if (m_Specification.ArraySize != 1)
        {
            // Easy case, since each mip level has a consistent number of slices.
            OB_ASSERT(z == 0, "[VkStagingImage] For an image array the depth must be 0.");
            OB_ASSERT(arraySlice < m_Specification.ArraySize, "[VkStagingImage] ArraySlice exceeds the amount of array slices in image.");
            OB_ASSERT(m_SliceRegions.size() == m_Specification.MipLevels * m_Specification.ArraySize, "");

            return m_SliceRegions[static_cast<size_t>(mipLevel) * m_Specification.ArraySize + arraySlice];
        }
        else
        {
            OB_ASSERT(arraySlice == 0, "[VkStagingImage] Must be first array slice.");
            OB_ASSERT(z == 0, "[VkStagingImage] Depth must be 0.");
            OB_ASSERT(m_SliceRegions.size() == m_Specification.MipLevels, "");

            return m_SliceRegions[mipLevel];
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanSampler::VulkanSampler(const Device& device, const SamplerSpecification& specs)
        : m_Device(*api_cast<const VulkanDevice*>(&device)), m_Specification(specs)
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

        VK_VERIFY(vkCreateSampler(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &samplerInfo, VulkanAllocator::GetCallbacks(), &m_Sampler));

        if constexpr (Information::Validation)
        {
            if (!m_Specification.DebugName.empty())
                m_Device.GetContext().SetDebugName(m_Sampler, VK_OBJECT_TYPE_SAMPLER, std::string(m_Specification.DebugName));
        }
    }

    VulkanSampler::~VulkanSampler()
    {
    }

}