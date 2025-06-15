#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/BindingsSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <Nano/Nano.hpp>

#include <span>
#include <variant>
#include <string_view>

namespace Nano::Graphics
{
    class Device;
    class BindingSetPool;
}

namespace Nano::Graphics::Internal
{

    class VulkanDevice;
    class VulkanBindingSetPool;

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

        // Getters
        //ShaderStage GetVisibility() const;
        inline bool IsBindless() const { return std::holds_alternative<BindlessLayoutSpecification>(m_Specification); }

        // Internal getters
        std::span<const BindingLayoutItem> GetBindingItems() const;

        inline VkDescriptorSetLayout GetVkDescriptorSetLayout() const { return m_Layout; }

        inline const std::vector<VkDescriptorSetLayoutBinding>& GetLayoutBindings() const { return m_LayoutBindings; }
        inline const std::vector<VkDescriptorPoolSize>& GetPoolSizeInfo() const { return m_PoolSizeInfo; }

    private:
        // Private methods
        void Finish(const VulkanDevice& device);

        // Private getters
        std::string_view GetDebugName() const;

    private:
        std::variant<BindingLayoutSpecification, BindlessLayoutSpecification> m_Specification;

        VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSetLayoutBinding> m_LayoutBindings = { };
        std::vector<VkDescriptorPoolSize> m_PoolSizeInfo = { };
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanBindingSet
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanBindingSet
    {
    public:
        // Constructor & Destructor
        VulkanBindingSet(BindingSetPool& pool);
        ~VulkanBindingSet();

        // Internal getters
        inline VkDescriptorSet GetVkDescriptorSet() const { return m_DescriptorSet; }

    private:
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

        // Internal methods
        VkDescriptorSet CreateDescriptorSet();

        // Internal getters
        inline VkDescriptorPool GetVkDescriptorPool() const { return m_DescriptorPool; }

    private:
        const VulkanDevice& m_Device;
        BindingSetPoolSpecification m_Specification;

        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;

        uint32_t m_CurrentDescriptor = 0;
        std::vector<VkDescriptorSet> m_DescriptorSets;
    };

}