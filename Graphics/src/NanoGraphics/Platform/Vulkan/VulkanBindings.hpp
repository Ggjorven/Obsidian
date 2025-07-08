#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/BindingsSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <Nano/Nano.hpp>

#include <span>
#include <variant>
#include <string_view>

namespace Nano::Graphics
{
    class Device;
    class Image;
    class Sampler;
    class Buffer;
    class BindingSetPool;
}

namespace Nano::Graphics::Internal
{

    class VulkanDevice;
    class VulkanBindingLayout;
    class VulkanBindingSet;
    class VulkanBindingSetPool;

#if defined(NG_API_VULKAN)
    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanBindingLayout
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanBindingLayout
    {
    public:
        // Constructors & Destructor
        VulkanBindingLayout(const Device& device, const BindingLayoutSpecification& specs);
        VulkanBindingLayout(const Device& device, const BindlessLayoutSpecification& specs);
        ~VulkanBindingLayout();

        // Methods
        const BindingLayoutItem& GetItem(uint32_t slot) const;

        // Getters
        inline bool IsBindless() const { return std::holds_alternative<BindlessLayoutSpecification>(m_Specification); }

        // Internal methods
        void UpdatePoolSizeInfosToMaxSets(uint32_t maxSets); // Note: Multiplies each descriptor count by the MaxSets

        // Internal getters
        BindingLayoutSpecification GetBindingLayoutSpecification() const { NG_ASSERT(!IsBindless(), "[VkBindingLayout] Can't retrieve BindingLayoutSpecification for bindless layout."); return std::get<BindingLayoutSpecification>(m_Specification); }

        std::span<const BindingLayoutItem> GetBindingItems() const;

        inline VkDescriptorSetLayout GetVkDescriptorSetLayout() const { return m_Layout; }

        inline const std::vector<VkDescriptorPoolSize>& GetPoolSizeInfo() const { return m_PoolSizeInfo; }

    private:
        // Private methods
        void Finish(const VulkanDevice& device, const std::vector<VkDescriptorSetLayoutBinding>& layoutBindings);

        // Private getters
        std::string_view GetDebugName() const;

    private:
        std::variant<BindingLayoutSpecification, BindlessLayoutSpecification> m_Specification;

        VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorPoolSize> m_PoolSizeInfo = { };
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanBindingSet
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanBindingSet
    {
    public:
        // Constructor & Destructor
        VulkanBindingSet(BindingSetPool& pool, const BindingSetSpecification& specs);
        ~VulkanBindingSet();

        // Methods
        void SetItem(uint32_t slot, Image& image, const ImageSubresourceSpecification& subresources, uint32_t arrayIndex);
        void SetItem(uint32_t slot, Sampler& sampler, uint32_t arrayIndex);
        void SetItem(uint32_t slot, Buffer& buffer, const BufferRange& range, uint32_t arrayIndex);

        // Getters
        inline const BindingSetSpecification& GetSpecification() const { return m_Specification; }

        // Internal getters
        inline VulkanBindingSetPool& GetVulkanBindingSetPool() { return m_Pool; }
        inline const VulkanBindingSetPool& GetVulkanBindingSetPool() const { return m_Pool; }

        inline VkDescriptorSet GetVkDescriptorSet() const { return m_DescriptorSet; }

    private:
        // Private methods
        void UploadImage(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorImageInfo>& imageInfos, Image& image, const ImageSubresourceSpecification& subresources, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex) const;
        void UploadSampler(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorImageInfo>& imageInfos, Sampler& sampler, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex) const;
        void UploadBuffer(std::vector<VkWriteDescriptorSet>& writes, std::vector<VkDescriptorBufferInfo>& bufferInfos, Buffer& buffer, const BufferRange& range, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex) const;
    
    private:
        VulkanBindingSetPool& m_Pool;
        BindingSetSpecification m_Specification;

        VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanBindingSetPool
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanBindingSetPool
    {
    public:
        // Constructor & Destructor
        VulkanBindingSetPool(const Device& device, const BindingSetPoolSpecification& specs);
        ~VulkanBindingSetPool();

        // Getters
        inline const BindingSetPoolSpecification& GetSpecification() const { return m_Specification; }

        // Internal methods
        VkDescriptorSet CreateDescriptorSet();

        // Internal getters
        inline const VulkanDevice& GetVulkanDevice() const { return m_Device; }

        inline VkDescriptorPool GetVkDescriptorPool() const { return m_DescriptorPool; }

    private:
        const VulkanDevice& m_Device;
        BindingSetPoolSpecification m_Specification;

        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;

        uint32_t m_CurrentDescriptor = 0;
        std::vector<VkDescriptorSet> m_DescriptorSets;
    };
#endif

}