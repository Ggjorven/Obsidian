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

        BindingLayoutSpecification& mSpecs = std::get<BindingLayoutSpecification>(m_Specification);

        // Sort the bindings by slot item from lowest to highest
        std::sort(mSpecs.Bindings.begin(), mSpecs.Bindings.end(), [](const BindingLayoutItem& a, const BindingLayoutItem& b) { return a.Slot < b.Slot; });

        std::span<const BindingLayoutItem> items = std::span<const BindingLayoutItem>(mSpecs.Bindings.begin(), mSpecs.Bindings.end());

        InitResourceCounts(items);
        CreateRootParameters(items);
    }

    Dx12BindingLayout::Dx12BindingLayout(const Device& device, const BindlessLayoutSpecification& specs)
        : m_Specification(specs)
    {
        (void)device;

        BindlessLayoutSpecification& mSpecs = std::get<BindlessLayoutSpecification>(m_Specification);
        
        // Sort the bindings by slot item from lowest to highest
        std::sort(mSpecs.Bindings.begin(), mSpecs.Bindings.end(), [](const BindingLayoutItem& a, const BindingLayoutItem& b) { return a.Slot < b.Slot; });

        std::span<const BindingLayoutItem> items = std::span<const BindingLayoutItem>(mSpecs.Bindings.begin(), mSpecs.Bindings.end());

        InitResourceCounts(items);
        CreateRootParameters(items);
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
    uint8_t Dx12BindingLayout::GetRegisterSpace() const
    {
        return std::visit([](auto&& obj) -> uint8_t
        {
            if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, BindingLayoutSpecification>)
                return obj.RegisterSpace;
            else if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, BindlessLayoutSpecification>)
                return obj.RegisterSpace;
        }, m_Specification);
    }

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
        // Note: We resize instead of reserve and emplace_back to be able to directly index
        // into the vector (which is faster than searching for the slot)
        if (!GetBindingSpecification().Bindings.empty())
            m_SlotToHeapOffset.resize(static_cast<size_t>(GetBindingSpecification().Bindings.back().Slot) + 1);

        // Map the counts & Map the offsets
        uint32_t offset = 0;
        std::unordered_map<ResourceType, uint32_t> counts = { };
        for (const auto& item : items) // Note: The items are sorted from lowest to highest slot in the constructor
        {
            // Map the counts
            if (!(counts.contains(item.Type)))
                counts[item.Type] = item.GetArraySize();
            else
                counts[item.Type] += item.GetArraySize();

            // Map the offsets
            m_SlotToHeapOffset[item.Slot] = offset;
            offset += item.GetArraySize();
        }

        // Add to vector
        m_ResourceCounts.reserve(counts.size());
        for (const auto& [type, count] : counts)
            m_ResourceCounts.emplace_back(type, count);
    }

    void Dx12BindingLayout::CreateRootParameters(std::span<const BindingLayoutItem> items)
    {
        m_SRVAndUAVAndCBVRanges.reserve(items.size());
        m_SamplerRanges.reserve(items.size());

        // Create ranges
        {
            uint8_t registerSpace = GetRegisterSpace();

            for (const auto& item : items)
            {
                switch (item.Type)
                {
                case ResourceType::TextureSRV:
                case ResourceType::StructuredBufferSRV:
                case ResourceType::DynamicStructuredBufferSRV:
                {
                    auto& [slot, stage, isDynamic, range] = m_SRVAndUAVAndCBVRanges.emplace_back();
                    
                    range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, item.GetArraySize(), item.Slot, registerSpace);
                    stage = item.Visibility;
                    slot = item.Slot;
                    isDynamic = (item.Type == ResourceType::DynamicStructuredBufferSRV);

                    break;
                }

                case ResourceType::TextureUAV:
                case ResourceType::StructuredBufferUAV:
                case ResourceType::DynamicStructuredBufferUAV:
                {
                    auto& [slot, stage, isDynamic, range] = m_SRVAndUAVAndCBVRanges.emplace_back();

                    range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, item.GetArraySize(), item.Slot, registerSpace);
                    stage = item.Visibility;
                    slot = item.Slot;
                    isDynamic = (item.Type == ResourceType::DynamicStructuredBufferUAV);

                    break;
                }

                case ResourceType::ConstantBuffer:
                case ResourceType::DynamicConstantBuffer:
                {
                    auto& [slot, stage, isDynamic, range] = m_SRVAndUAVAndCBVRanges.emplace_back();

                    range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, item.GetArraySize(), item.Slot, registerSpace);
                    stage = item.Visibility;
                    slot = item.Slot;
                    isDynamic = (item.Type == ResourceType::DynamicConstantBuffer);

                    break;
                }

                case ResourceType::Sampler:
                {
                    auto& [slot, stage, range] = m_SamplerRanges.emplace_back();

                    range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, item.GetArraySize(), item.Slot, registerSpace);
                    stage = item.Visibility;
                    slot = item.Slot;

                    break;
                }

                // Note: PushConstants are not needed in the descriptor heap
                // they are stored in fast-accessible gpu memory.
                case ResourceType::PushConstants:
                    break;

                default:
                    NG_UNREACHABLE();
                    break;
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12BindingSet::Dx12BindingSet(BindingSetPool& pool, const BindingSetSpecification& specs)
        : m_Pool(*api_cast<Dx12BindingSetPool*>(&pool)), m_Specification(specs)
    {
        // Get indices
        {
            // Note: Modifies m_XXXBeginIndex variables.
            m_Pool.GetNextSetIndices(m_SRVAndUAVAndCBVBeginIndex, m_SamplerBeginIndex);
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

        NG_ASSERT(((item.Type == ResourceType::TextureSRV) || (item.Type == ResourceType::TextureUAV)), "[Dx12BindingSet] When uploading an image the ResourceType must be TextureSRV or TextureUAV.");

        Dx12Image& dxImage = *api_cast<Dx12Image*>(&image);
        DescriptorHeapIndex index = m_SRVAndUAVAndCBVBeginIndex + dxLayout.GetSlotToHeapOffset(slot) + arrayIndex;

        switch (item.Type)
        {
        case ResourceType::TextureSRV:
            m_Pool.GetDx12Device().GetResources().GetSRVAndUAVAndCBVHeap().CreateSRV(index, image.GetSpecification(), subresources, dxImage.GetD3D12Resource().Get());
            break;

        case ResourceType::TextureUAV:
            m_Pool.GetDx12Device().GetResources().GetSRVAndUAVAndCBVHeap().CreateUAV(index, image.GetSpecification(), subresources, dxImage.GetD3D12Resource().Get());
            break;

        default:
            NG_UNREACHABLE();
            break;
        }
    }

    void Dx12BindingSet::SetItem(uint32_t slot, Sampler& sampler, uint32_t arrayIndex)
    {
        Dx12BindingLayout& dxLayout = *api_cast<Dx12BindingLayout*>(m_Pool.GetSpecification().Layout);
        const auto& item = dxLayout.GetItem(slot);

        NG_ASSERT((item.Type == ResourceType::Sampler), "[Dx12BindingSet] When uploading a sampler the ResourceType must be Sampler.");

        //Dx12Sampler& dxSampler = *api_cast<Dx12Sampler*>(&sampler);
        DescriptorHeapIndex index = m_SamplerBeginIndex + dxLayout.GetSlotToHeapOffset(slot) + arrayIndex;

        m_Pool.GetDx12Device().GetResources().GetSamplerHeap().CreateSampler(index, sampler.GetSpecification());
    }

    void Dx12BindingSet::SetItem(uint32_t slot, Buffer& buffer, const BufferRange& range, uint32_t arrayIndex)
    {
        // Note: I don't know if arrayIndex is actually usable for buffers, but for now
        // it exists, it might not translate to dx12/hlsl. Careful with this.

        Dx12BindingLayout& dxLayout = *api_cast<Dx12BindingLayout*>(m_Pool.GetSpecification().Layout);
        const auto& item = dxLayout.GetItem(slot);

        NG_ASSERT(((item.Type == ResourceType::StructuredBufferSRV) || (item.Type == ResourceType::StructuredBufferUAV) || (item.Type == ResourceType::ConstantBuffer)), "[Dx12BindingSet] When uploading a buffer the ResourceType must be StructuredBufferSRV, StructuredBufferUAV or ConstantBuffer.");

        Dx12Buffer& dxBuffer = *api_cast<Dx12Buffer*>(&buffer);
        DescriptorHeapIndex index = m_SRVAndUAVAndCBVBeginIndex + dxLayout.GetSlotToHeapOffset(slot) + arrayIndex;
        auto& heap = m_Pool.GetDx12Device().GetResources().GetSRVAndUAVAndCBVHeap();

        switch (item.Type)
        {
        case ResourceType::StructuredBufferSRV:
            heap.CreateSRV(index, buffer.GetSpecification(), range, ResourceType::StructuredBufferSRV, dxBuffer.GetD3D12Resource().Get());
            break;

        case ResourceType::StructuredBufferUAV:
            heap.CreateUAV(index, buffer.GetSpecification(), range, ResourceType::StructuredBufferUAV, dxBuffer.GetD3D12Resource().Get());
            break;

        case ResourceType::ConstantBuffer:
            heap.CreateCBV(index, buffer.GetSpecification(), ResourceType::ConstantBuffer, dxBuffer.GetD3D12Resource().Get());
            break;

        case ResourceType::DynamicStructuredBufferSRV:
        {
            NG_ASSERT((range.Size == BufferRange::FullSize), "[Dx12BindingSet] Dynamic buffers require a buffer range of FullSize.");
            NG_ASSERT((range.Offset == 0), "[Dx12BindingSet] Dynamic buffers require no buffer range offset.");

            BufferRange dynamicRange = {};
            dynamicRange.Offset = 0;
            dynamicRange.Size = buffer.GetSpecification().Stride;

            for (uint32_t i = 0; i < item.Size; i++)
            {
                heap.CreateSRV(index, buffer.GetSpecification(), dynamicRange, ResourceType::DynamicStructuredBufferSRV, dxBuffer.GetD3D12Resource().Get());
                dynamicRange.Offset = Nano::Memory::AlignOffset(dynamicRange.Offset + buffer.GetSpecification().Stride, buffer.GetAlignment());
            }
            break;
        }
        case ResourceType::DynamicStructuredBufferUAV:
        {
            NG_ASSERT((range.Size == BufferRange::FullSize), "[Dx12BindingSet] Dynamic buffers require a buffer range of FullSize.");
            NG_ASSERT((range.Offset == 0), "[Dx12BindingSet] Dynamic buffers require no buffer range offset.");

            BufferRange dynamicRange = {};
            dynamicRange.Offset = 0;
            dynamicRange.Size = buffer.GetSpecification().Stride;

            for (uint32_t i = 0; i < item.Size; i++)
            {
                heap.CreateUAV(index, buffer.GetSpecification(), dynamicRange, ResourceType::DynamicStructuredBufferUAV, dxBuffer.GetD3D12Resource().Get());
                dynamicRange.Offset = Nano::Memory::AlignOffset(dynamicRange.Offset + buffer.GetSpecification().Stride, buffer.GetAlignment());
            }
            break;
        }
        case ResourceType::DynamicConstantBuffer:
        {
            NG_ASSERT((range.Size == BufferRange::FullSize), "[Dx12BindingSet] Dynamic buffers require a buffer range of FullSize.");
            NG_ASSERT((range.Offset == 0), "[Dx12BindingSet] Dynamic buffers require no buffer range offset.");

            ID3D12Resource* resource = dxBuffer.GetD3D12Resource().Get();
            size_t alignedSize = Nano::Memory::AlignOffset(buffer.GetSpecification().Stride, buffer.GetAlignment());

            D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
            desc.SizeInBytes = static_cast<uint32_t>(buffer.GetSpecification().Stride);
            desc.BufferLocation = resource->GetGPUVirtualAddress();

            for (uint32_t i = 0; i < item.Size; i++)
            {
                heap.CreateCBV(index, desc, resource);
                desc.BufferLocation += alignedSize;
            }
            break;
        }

        default:
            NG_UNREACHABLE();
            break;
        }
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
                switch (type)
                {
                case ResourceType::TextureSRV:
                case ResourceType::StructuredBufferSRV:
                case ResourceType::DynamicStructuredBufferSRV:
                // Note: These are all in the descriptor heap
                case ResourceType::TextureUAV:
                case ResourceType::StructuredBufferUAV:
                case ResourceType::DynamicStructuredBufferUAV:
                // Note: These are all in the descriptor heap
                case ResourceType::ConstantBuffer:
                case ResourceType::DynamicConstantBuffer:
                case ResourceType::PushConstants:
                    m_SRVAndUAVAndCBVCountPerSet += count; // Note: Count == ArraySize or Dynamic Element Count
                    break;

                case ResourceType::Sampler:
                    m_SamplerCountPerSet += count;
                    break;

                default:
                    NG_UNREACHABLE();
                    break;
                };
            }
        }

        // Allocating
        {
            m_SRVAndUAVAndCBVBeginIndex = m_Device.GetResources().GetSRVAndUAVAndCBVHeap().GetNextPoolIndex(m_Specification.SetAmount, m_SRVAndUAVAndCBVCountPerSet + m_SamplerCountPerSet);
            m_SamplerBeginIndex = m_Device.GetResources().GetSamplerHeap().GetNextPoolIndex(m_Specification.SetAmount, m_SRVAndUAVAndCBVCountPerSet + m_SamplerCountPerSet);
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
        
        outSRVAndUAVBeginIndex = m_SRVAndUAVAndCBVBeginIndex + (m_CurrentSet * (m_SRVAndUAVAndCBVCountPerSet + m_SamplerCountPerSet));
        outSamplerBeginIndex = m_SamplerBeginIndex + (m_CurrentSet * (m_SRVAndUAVAndCBVCountPerSet + m_SamplerCountPerSet));

        m_CurrentSet++;
    }

}