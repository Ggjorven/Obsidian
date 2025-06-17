#include "ngpch.h"
#include "VulkanBindings.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"

#include <type_traits>

namespace Nano::Graphics::Internal
{

    static_assert(std::is_same_v<Device::Type, VulkanDevice>, "Current Device::Type is not VulkanDevice and Vulkan source code is being compiled.");

    namespace
    {

        ////////////////////////////////////////////////////////////////////////////////////
        // ResourceTypeToLayoutsAndUsageMapping
        ////////////////////////////////////////////////////////////////////////////////////
        struct ResourceTypeToLayoutsAndUsageMapping
        {
        public:
            ResourceType Type;

            VkImageLayout VulkanImageLayout;
            VkImageUsageFlags VulkanImageUsage;
        };

        ////////////////////////////////////////////////////////////////////////////////////
        // ResourceTypeToLayoutsAndUsageMapping array
        ////////////////////////////////////////////////////////////////////////////////////
        inline constexpr static auto g_ResourceTypeToLayoutsAndUsageMapping = std::to_array<ResourceTypeToLayoutsAndUsageMapping>({
            // ResourceType                 VulkanImageLayout                           VulkanImageUsage
            { ResourceType::Image,          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,   VK_IMAGE_USAGE_SAMPLED_BIT },
            { ResourceType::ImageUnordered, VK_IMAGE_LAYOUT_GENERAL,                    VK_IMAGE_USAGE_STORAGE_BIT },
        });

    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructors & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanBindingLayout::VulkanBindingLayout(const Device& device, const BindingLayoutSpecification& specs)
        : m_Specification(specs)
    {
        //VkShaderStageFlags shaderStageFlags = ShaderStageToVkShaderStageFlags(specs.Visibility);

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.reserve(specs.Bindings.size());

        for (const BindingLayoutItem& item : specs.Bindings)
        {
            if (item.Type == ResourceType::PushConstants) // Note: No descriptor needed for PushConstants
                continue;

            VkDescriptorType descriptorType = ResourceTypeToVkDescriptorType(item.Type);
            uint32_t descriptorCount = item.Size;

            VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
            descriptorSetLayoutBinding.binding = item.Slot;
            descriptorSetLayoutBinding.descriptorCount = descriptorCount;
            descriptorSetLayoutBinding.descriptorType = descriptorType;
            descriptorSetLayoutBinding.stageFlags = ShaderStageToVkShaderStageFlags(item.Visibility);
            
            layoutBindings.push_back(descriptorSetLayoutBinding);
        }

        Finish(*reinterpret_cast<const VulkanDevice*>(&device), layoutBindings);
    }

    VulkanBindingLayout::VulkanBindingLayout(const Device& device, const BindlessLayoutSpecification& specs)
        : m_Specification(specs)
    {
        //VkShaderStageFlags shaderStageFlags = ShaderStageToVkShaderStageFlags(specs.Visibility);

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.reserve(specs.RegisterSpaces.size());
        
        uint32_t bindingPoint = 0;
        uint32_t arraySize = specs.MaxCapacity;

        for (const BindingLayoutItem& item : specs.RegisterSpaces)
        {
            VkDescriptorType descriptorType = ResourceTypeToVkDescriptorType(item.Type);

            VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
            descriptorSetLayoutBinding.binding = bindingPoint++; // Note: We increase the bindingPoint
            descriptorSetLayoutBinding.descriptorCount = arraySize;
            descriptorSetLayoutBinding.descriptorType = descriptorType;
            descriptorSetLayoutBinding.stageFlags = ShaderStageToVkShaderStageFlags(item.Visibility);

            layoutBindings.push_back(descriptorSetLayoutBinding);
        }

        Finish(*reinterpret_cast<const VulkanDevice*>(&device), layoutBindings);
    }

    VulkanBindingLayout::~VulkanBindingLayout()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanBindingLayout::UpdatePoolSizeInfosToMaxSets(uint32_t maxSets)
    {
        for (auto& poolSize : m_PoolSizeInfo)
            poolSize.descriptorCount *= maxSets;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Getters
    ////////////////////////////////////////////////////////////////////////////////////
    //ShaderStage VulkanBindingLayout::GetVisibility() const
    //{
    //    return std::visit([](auto&& obj) -> ShaderStage
    //    { 
    //        if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, BindingLayoutSpecification>)
    //            return obj.Visibility;
    //        else if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, BindlessLayoutSpecification>)
    //            return obj.Visibility;
    //    }, m_Specification);
    //}

    ////////////////////////////////////////////////////////////////////////////////////
    // Internal getters
    ////////////////////////////////////////////////////////////////////////////////////
    std::span<const BindingLayoutItem> VulkanBindingLayout::GetBindingItems() const
    {
        return std::visit([](auto&& obj) -> std::span<const BindingLayoutItem>
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, BindingLayoutSpecification>)
                return std::span<const BindingLayoutItem>(obj.Bindings);
            else if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, BindlessLayoutSpecification>)
                return std::span<const BindingLayoutItem>(obj.RegisterSpaces);
        }, m_Specification);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanBindingLayout::Finish(const VulkanDevice& device, const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings)
    {
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        //descriptorSetLayoutCreateInfo.flags = (IsBindless() ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT : 0);
        descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        descriptorSetLayoutCreateInfo.pBindings = layoutBindings.data();

        std::vector<VkDescriptorBindingFlags> bindlessFlags(layoutBindings.size(), VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT);
        VkDescriptorSetLayoutBindingFlagsCreateInfo extendedCreateInfo = {}; // Note: For bindless
        extendedCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        extendedCreateInfo.bindingCount = static_cast<uint32_t>(bindlessFlags.size());
        extendedCreateInfo.pBindingFlags = bindlessFlags.data();

        // Only set the bindless if we have bindless
        descriptorSetLayoutCreateInfo.pNext = (IsBindless() ? &extendedCreateInfo : nullptr);

        VK_VERIFY(vkCreateDescriptorSetLayout(device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &descriptorSetLayoutCreateInfo, VulkanAllocator::GetCallbacks(), &m_Layout));
        
        // Count number of descriptors per type
        std::unordered_map<VkDescriptorType, uint32_t> poolSizeMap;
        for (const auto& layoutBinding : layoutBindings)
        {
            // Initialize the mapping
            if (!poolSizeMap.contains(layoutBinding.descriptorType))
                poolSizeMap[layoutBinding.descriptorType] = 0;

            poolSizeMap[layoutBinding.descriptorType] += layoutBinding.descriptorCount;
        }

        // Compute descriptor pool size info
        for (const auto& [type, count] : poolSizeMap)
        {
            // Note: Just for myself, the poolSizes is just the amount of elements of a certain type to able to allocate per pool
            if (count > 0)
                m_PoolSizeInfo.emplace_back(type, count);
        }

        if constexpr (VulkanContext::Validation)
        {
            if (!GetDebugName().empty())
                device.GetContext().SetDebugName(m_Layout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, std::string(GetDebugName()));
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////////////////////////////////////////////////
    std::string_view VulkanBindingLayout::GetDebugName() const
    {
        return std::visit([](auto&& obj) -> std::string_view
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, BindingLayoutSpecification>)
                return obj.DebugName;
            else if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, BindlessLayoutSpecification>)
                return obj.DebugName;
        }, m_Specification);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanBindingSet::VulkanBindingSet(BindingSetPool& pool)
        : m_Device(reinterpret_cast<VulkanBindingSetPool*>(&pool)->GetVulkanDevice()), m_DescriptorSet(reinterpret_cast<VulkanBindingSetPool*>(&pool)->CreateDescriptorSet())
    {
    }

    VulkanBindingSet::~VulkanBindingSet()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanBindingSet::Upload(Image& image, const ImageSubresourceSpecification& subresources, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex)
    {
        NG_ASSERT(((resourceType == ResourceType::Image) || (resourceType == ResourceType::ImageUnordered)), "[VkBindingSet] When uploading an image the ResourceType must be Image or ImageUnordered.");

        VulkanImage& vulkanImage = *reinterpret_cast<VulkanImage*>(&image);
        ImageSubresourceSpecification resSubresources = ResolveImageSubresouce(subresources, image.GetSpecification(), false);

        VkImageLayout imageLayout = g_ResourceTypeToLayoutsAndUsageMapping[static_cast<size_t>(resourceType) - static_cast<size_t>(ResourceType::Image)].VulkanImageLayout;
        VkImageUsageFlags imageUsage = g_ResourceTypeToLayoutsAndUsageMapping[static_cast<size_t>(resourceType) - static_cast<size_t>(ResourceType::Image)].VulkanImageUsage;

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = imageLayout;
        imageInfo.imageView = vulkanImage.GetSubresourceView(resSubresources, image.GetSpecification().Dimension, image.GetSpecification().ImageFormat, imageUsage, FormatToImageSubresourceViewType(image.GetSpecification().ImageFormat)).GetVkImageView();
        
        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSet;
        descriptorWrite.dstBinding = slot;
        descriptorWrite.dstArrayElement = arrayIndex;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;
        
        vkUpdateDescriptorSets(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), 1, &descriptorWrite, 0, nullptr);
    }

    void VulkanBindingSet::Upload(Sampler& sampler, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex)
    {
        (void)resourceType;
        NG_ASSERT((resourceType == ResourceType::Sampler), "[VkBindingSet] When uploading a sampler the ResourceType must be Sampler.");
        
        VulkanSampler& vulkanSampler = *reinterpret_cast<VulkanSampler*>(&sampler);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.sampler = vulkanSampler.GetVkSampler();

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSet;
        descriptorWrite.dstBinding = slot;
        descriptorWrite.dstArrayElement = arrayIndex;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), 1, &descriptorWrite, 0, nullptr);
    }

    void VulkanBindingSet::Upload(Buffer& buffer, const BufferRange& range, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex)
    {
        NG_ASSERT(((resourceType == ResourceType::StorageBuffer) || (resourceType == ResourceType::StorageBufferUnordered) || (resourceType == ResourceType::UniformBuffer)), "[VkBindingSet] When uploading a buffer the ResourceType must be StorageBuffer, StorageBufferUnordered or UniformBuffer.");

        VulkanBuffer& vulkanBuffer = *reinterpret_cast<VulkanBuffer*>(&buffer);
        BufferRange resRange = ResolveBufferRange(range, buffer.GetSpecification());

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = vulkanBuffer.GetVkBuffer();
        bufferInfo.offset = resRange.Offset;
        bufferInfo.range = resRange.Size;

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSet;
        descriptorWrite.dstBinding = slot;
        descriptorWrite.dstArrayElement = arrayIndex;
        descriptorWrite.descriptorType = ResourceTypeToVkDescriptorType(resourceType);
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), 1, &descriptorWrite, 0, nullptr);
    }

    void VulkanBindingSet::UploadList(std::span<const BindingSetUploadable> uploadables)
    {
        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(uploadables.size());
        std::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.reserve(uploadables.size());
        std::vector<VkDescriptorBufferInfo> bufferInfos;
        bufferInfos.reserve(uploadables.size());

        for (const auto& uploadable : uploadables)
        {
            std::visit([&](auto&& obj)
            {
                if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, Image*>)
                {
                    NG_ASSERT((std::holds_alternative<ImageSubresourceSpecification>(uploadable.Range)), "[VkBindingSet] Image passed in as uploadable, but no complimentary subresource passed in.");
                    UploadImage(writes, imageInfos, *obj, std::get<ImageSubresourceSpecification>(uploadable.Range), uploadable.Type, uploadable.Slot, uploadable.ArrayIndex);
                }
                else if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, Sampler*>)
                {
                    UploadSampler(writes, imageInfos, *obj, uploadable.Type, uploadable.Slot, uploadable.ArrayIndex);
                }   
                else if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, Buffer*>)
                {
                    NG_ASSERT((std::holds_alternative<BufferRange>(uploadable.Range)), "[VkBindingSet] Buffer passed in as uploadable, but no complimentary bufferrange passed in.");
                    UploadBuffer(writes, bufferInfos, *obj, std::get<BufferRange>(uploadable.Range), uploadable.Type, uploadable.Slot, uploadable.ArrayIndex);
                }
            }, uploadable.Element);
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Private methods // Note: I am aware of the code duplications, but this is the fastest approach.
    ////////////////////////////////////////////////////////////////////////////////////
    void VulkanBindingSet::UploadImage(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorImageInfo>& imageInfos, Image& image, const ImageSubresourceSpecification& subresources, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex) const
    {
        NG_ASSERT(((resourceType == ResourceType::Image) || (resourceType == ResourceType::ImageUnordered)), "[VkBindingSet] When uploading an image the ResourceType must be Image or ImageUnordered.");
    
        VulkanImage& vulkanImage = *reinterpret_cast<VulkanImage*>(&image);
        ImageSubresourceSpecification resSubresources = ResolveImageSubresouce(subresources, image.GetSpecification(), false);

        VkImageLayout imageLayout = g_ResourceTypeToLayoutsAndUsageMapping[static_cast<size_t>(resourceType) - static_cast<size_t>(ResourceType::Image)].VulkanImageLayout;
        VkImageUsageFlags imageUsage = g_ResourceTypeToLayoutsAndUsageMapping[static_cast<size_t>(resourceType) - static_cast<size_t>(ResourceType::Image)].VulkanImageUsage;

        VkDescriptorImageInfo& imageInfo = imageInfos.emplace_back();
        imageInfo.imageLayout = imageLayout;
        imageInfo.imageView = vulkanImage.GetSubresourceView(resSubresources, image.GetSpecification().Dimension, image.GetSpecification().ImageFormat, imageUsage, FormatToImageSubresourceViewType(image.GetSpecification().ImageFormat)).GetVkImageView();

        VkWriteDescriptorSet& descriptorWrite = writes.emplace_back();
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSet;
        descriptorWrite.dstBinding = slot;
        descriptorWrite.dstArrayElement = arrayIndex;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;
    }

    void VulkanBindingSet::UploadSampler(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorImageInfo>& imageInfos, Sampler& sampler, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex) const
    {
        (void)resourceType;
        NG_ASSERT((resourceType == ResourceType::Sampler), "[VkBindingSet] When uploading a sampler the ResourceType must be Sampler.");
    
        VulkanSampler& vulkanSampler = *reinterpret_cast<VulkanSampler*>(&sampler);

        VkDescriptorImageInfo& imageInfo = imageInfos.emplace_back();
        imageInfo.sampler = vulkanSampler.GetVkSampler();

        VkWriteDescriptorSet& descriptorWrite = writes.emplace_back();
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSet;
        descriptorWrite.dstBinding = slot;
        descriptorWrite.dstArrayElement = arrayIndex;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;
    }

    void VulkanBindingSet::UploadBuffer(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorBufferInfo>& bufferInfos, Buffer& buffer, const BufferRange& range, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex) const
    {
        NG_ASSERT(((resourceType == ResourceType::StorageBuffer) || (resourceType == ResourceType::StorageBufferUnordered) || (resourceType == ResourceType::UniformBuffer)), "[VkBindingSet] When uploading a buffer the ResourceType must be StorageBuffer, StorageBufferUnordered or UniformBuffer.");
    
        VulkanBuffer& vulkanBuffer = *reinterpret_cast<VulkanBuffer*>(&buffer);
        BufferRange resRange = ResolveBufferRange(range, buffer.GetSpecification());

        VkDescriptorBufferInfo& bufferInfo = bufferInfos.emplace_back();
        bufferInfo.buffer = vulkanBuffer.GetVkBuffer();
        bufferInfo.offset = resRange.Offset;
        bufferInfo.range = resRange.Size;

        VkWriteDescriptorSet& descriptorWrite = writes.emplace_back();
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSet;
        descriptorWrite.dstBinding = slot;
        descriptorWrite.dstArrayElement = arrayIndex;
        descriptorWrite.descriptorType = ResourceTypeToVkDescriptorType(resourceType);
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanBindingSetPool::VulkanBindingSetPool(const Device& device, const BindingSetPoolSpecification& specs)
        : m_Device(*reinterpret_cast<const VulkanDevice*>(&device)), m_Specification(specs), m_DescriptorSets(static_cast<size_t>(m_Specification.SetAmount))
    {
        NG_ASSERT(m_Specification.Layout, "[VkBindingSetPool] No valid BindingLayout passed in.");
        NG_ASSERT(m_Specification.SetAmount > 0, "[VkBindingSetPool] SetAmount must be non-zero.");

        VulkanBindingLayout& bindingLayout = *reinterpret_cast<VulkanBindingLayout*>(m_Specification.Layout);
        bindingLayout.UpdatePoolSizeInfosToMaxSets(specs.SetAmount); // Update the infos.

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(bindingLayout.GetPoolSizeInfo().size());
        poolInfo.pPoolSizes = bindingLayout.GetPoolSizeInfo().data();
        poolInfo.maxSets = m_Specification.SetAmount;
        poolInfo.flags = (bindingLayout.IsBindless() ? VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT : 0); // For bindless support

        VK_VERIFY(vkCreateDescriptorPool(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &poolInfo, VulkanAllocator::GetCallbacks(), &m_DescriptorPool));
    
        if constexpr (VulkanContext::Validation)
        {
            if (!m_Specification.DebugName.empty())
                m_Device.GetContext().SetDebugName(m_DescriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, std::string(m_Specification.DebugName));
        }
    }

    VulkanBindingSetPool::~VulkanBindingSetPool()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Internal methods
    ////////////////////////////////////////////////////////////////////////////////////
    VkDescriptorSet VulkanBindingSetPool::CreateDescriptorSet()
    {
        NG_ASSERT((m_Specification.SetAmount > m_CurrentDescriptor), "[VkBindingSetPool] Cannot allocate more descriptor sets than the specification's count specified.");
        
        if (m_DescriptorSets[0] != VK_NULL_HANDLE) // Note: Early return if already allocated and return from cache
            return m_DescriptorSets[m_CurrentDescriptor++];

        VulkanBindingLayout& bindingLayout = *reinterpret_cast<VulkanBindingLayout*>(m_Specification.Layout);

        std::vector<VkDescriptorSetLayout> layouts(static_cast<size_t>(m_Specification.SetAmount), bindingLayout.GetVkDescriptorSetLayout());
        std::vector<uint32_t> maxBindings;
        maxBindings.reserve(static_cast<size_t>(m_Specification.SetAmount));

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = m_Specification.SetAmount;
        allocInfo.pSetLayouts = layouts.data();

        for (const auto& item : bindingLayout.GetBindingItems())
        {
            if (item.Type == ResourceType::PushConstants)
                continue;

            NG_ASSERT((item.Size != 0), "[VkBindingSetPool] BindingLayoutItem.Size == 0.");

            uint32_t maxBinding = item.Size - 1;
            maxBindings.push_back(maxBinding);
        }

        // Set max bindings when there is a bindless descriptor found
        VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countInfo = {};
        countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
        countInfo.descriptorSetCount = m_Specification.SetAmount;
        countInfo.pDescriptorCounts = maxBindings.data();

        allocInfo.pNext = (bindingLayout.IsBindless() ? &countInfo : nullptr); // Note: Add max bindings when Bindless

        VK_VERIFY(vkAllocateDescriptorSets(m_Device.GetContext().GetVulkanLogicalDevice().GetVkDevice(), &allocInfo, m_DescriptorSets.data()));

        if constexpr (VulkanContext::Validation)
        {
            if (!m_Specification.DebugName.empty())
            {
                for (size_t i = 0; i < m_DescriptorSets.size(); i++)
                    m_Device.GetContext().SetDebugName(m_DescriptorSets[i], VK_OBJECT_TYPE_DESCRIPTOR_SET, std::format("DescriptorSet({0}) for: {1}", i, m_Specification.DebugName));
            }
        }

        return m_DescriptorSets[m_CurrentDescriptor++];
    }

}