#include "ngpch.h"
#include "Dx12Bindings.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12Device.hpp"

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructors & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12BindingLayout::Dx12BindingLayout(const Device& device, const BindingLayoutSpecification& specs)
        : m_Specification(specs)
    {
        (void)device;
        InitResourceCounts(std::span<const BindingLayoutItem>(specs.Bindings.begin(), specs.Bindings.end()));
    }

    Dx12BindingLayout::Dx12BindingLayout(const Device& device, const BindlessLayoutSpecification& specs)
        : m_Specification(specs)
    {
        (void)device;
        InitResourceCounts(std::span<const BindingLayoutItem>(specs.Bindings.begin(), specs.Bindings.end()));
    }

    Dx12BindingLayout::~Dx12BindingLayout()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    const BindingLayoutItem& Dx12BindingLayout::GetItem(uint32_t slot) const
    {
        std::span<const BindingLayoutItem> items = GetBindingItems();

        for (const auto& item : items)
        {
            if (slot == item.Slot)
                return item;
        }

        NG_ASSERT(false, "[Dx12BindingLayout] Trying to retrieve a bindingitem from slot {0} which wasn't passed into the bindinglayout.", slot);
        return *static_cast<const BindingLayoutItem*>(nullptr);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Internal getters
    ////////////////////////////////////////////////////////////////////////////////////
    std::span<const BindingLayoutItem> Dx12BindingLayout::GetBindingItems() const
    {
        return std::visit([](auto&& obj) -> std::span<const BindingLayoutItem>
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, BindingLayoutSpecification>)
                return std::span<const BindingLayoutItem>(obj.Bindings);
            else if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, BindlessLayoutSpecification>)
                return std::span<const BindingLayoutItem>(obj.Bindings);
        }, m_Specification);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12BindingLayout::InitResourceCounts(std::span<const BindingLayoutItem> items)
    {
        // Map the counts
        std::unordered_map<ResourceType, uint32_t> counts = { };
        for (const auto& item : items)
        {
            if (!(counts.contains(item.Type)))
                counts[item.Type] = item.GetArraySize();
            else
                counts[item.Type] += item.GetArraySize();
        }

        // Add to vector
        m_ResourceCounts.reserve(counts.size());
        for (const auto& [type, count] : counts)
            m_ResourceCounts.emplace_back(type, count);
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor // TODO: Implement descriptorsets
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12BindingSet::Dx12BindingSet(BindingSetPool& pool, const BindingSetSpecification& specs)
        : m_Pool(*api_cast<Dx12BindingSetPool*>(&pool)), m_Specification(specs)
    {
        // Get indices
        {
            // Modifies m_XXXBeginIndex variables.
            m_Pool.GetNextSetIndices(m_SRVAndUAVBeginIndex, m_SamplerBeginIndex);
        }
    }

    Dx12BindingSet::~Dx12BindingSet()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12BindingSet::SetItem(uint32_t slot, Image& image, const ImageSubresourceSpecification& subresources, uint32_t arrayIndex)
    {
        Dx12BindingLayout& dxLayout = *api_cast<Dx12BindingLayout*>(m_Pool.GetSpecification().Layout);
        const auto& item = dxLayout.GetItem(slot);

        NG_ASSERT(((item.Type == ResourceType::Image) || (item.Type == ResourceType::ImageUnordered)), "[Dx12BindingSet] When uploading an image the ResourceType must be Image or ImageUnordered.");

        // TODO: ...
    }

    void Dx12BindingSet::SetItem(uint32_t slot, Sampler& sampler, uint32_t arrayIndex)
    {
        Dx12BindingLayout& dxLayout = *api_cast<Dx12BindingLayout*>(m_Pool.GetSpecification().Layout);
        const auto& item = dxLayout.GetItem(slot);

        NG_ASSERT((item.Type == ResourceType::Sampler), "[Dx12BindingSet] When uploading a sampler the ResourceType must be Sampler.");

        // TODO: ...
    }

    void Dx12BindingSet::SetItem(uint32_t slot, Buffer& buffer, const BufferRange& range, uint32_t arrayIndex)
    {
        Dx12BindingLayout& dxLayout = *api_cast<Dx12BindingLayout*>(m_Pool.GetSpecification().Layout);
        const auto& item = dxLayout.GetItem(slot);

        NG_ASSERT(((item.Type == ResourceType::StorageBuffer) || (item.Type == ResourceType::StorageBufferUnordered) || (item.Type == ResourceType::UniformBuffer) || (item.Type == ResourceType::DynamicUniformBuffer)), "[Dx12BindingSet] When uploading a buffer the ResourceType must be StorageBuffer, StorageBufferUnordered, UniformBuffer or DynamicUniformBuffer.");

        // TODO: ...
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12BindingSetPool::Dx12BindingSetPool(const Device& device, const BindingSetPoolSpecification& specs)
        : m_Device(*api_cast<const Dx12Device*>(&device)), m_Specification(specs)
    {
        // Counting
        {
            Dx12BindingLayout& dxLayout = *api_cast<Dx12BindingLayout*>(specs.Layout);
            for (const auto& [type, count] : dxLayout.GetResourceCounts())
            {
                // FUTURE TODO: Dx12 types
                switch (type)
                {
                case ResourceType::Image:
                case ResourceType::StorageBuffer:
                case ResourceType::UniformBuffer:

                case ResourceType::ImageUnordered:
                case ResourceType::StorageBufferUnordered:
                    m_SRVAndUAVCountPerSet += count;
                    break;

                case ResourceType::Sampler:
                    m_SamplerCountPerSet += count;
                    break;

                case ResourceType::PushConstants:
                    // TODO: Pushconstants don't do anything on dx12
                    break;

                default: // TODO: Dynamic on dx12?
                    NG_UNREACHABLE();
                    break;
                };
            }
        }

        // Allocating
        {
            m_SRVAndUAVBeginIndex = m_Device.GetResources().GetSRVAndUAVHeap().GetNextPoolIndex(m_Specification.SetAmount, m_SRVAndUAVCountPerSet);
            m_SRVAndUAVBeginIndex = m_Device.GetResources().GetSamplerHeap().GetNextPoolIndex(m_Specification.SetAmount, m_SamplerCountPerSet);
        }
    }

    Dx12BindingSetPool::~Dx12BindingSetPool()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Internal methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12BindingSetPool::GetNextSetIndices(DescriptorHeapIndex& outSRVAndUAVBeginIndex, DescriptorHeapIndex& outSamplerBeginIndex)
    {
        NG_ASSERT(((m_CurrentSet + 1) <= m_Specification.SetAmount), "[Dx12BindingSetPool] Using more bindingsets than specified in specification.");
        
        outSRVAndUAVBeginIndex = m_SRVAndUAVBeginIndex + (m_CurrentSet * m_SRVAndUAVCountPerSet);
        outSamplerBeginIndex = m_SamplerBeginIndex + (m_CurrentSet * m_SamplerCountPerSet);

        m_CurrentSet++;
    }

}