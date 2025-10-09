#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/API.hpp"
#include "Obsidian/Renderer/BindingsSpec.hpp"
#include "Obsidian/Renderer/ImageSpec.hpp"

#include "Obsidian/Platform/Dx12/Dx12.hpp"
#include "Obsidian/Platform/Dx12/Dx12Descriptors.hpp"

#include <Nano/Nano.hpp>

#include <span>
#include <variant>
#include <string_view>

namespace Obsidian
{
    class Device;
    class Image;
    class Sampler;
    class Buffer;
    class BindingSetPool;
}

namespace Obsidian::Internal
{

    class Dx12Device;
    class Dx12BindingLayout;
    class Dx12BindingSet;
    class Dx12BindingSetPool;

#if defined(OB_API_DX12)
    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12BindingLayout
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12BindingLayout
    {
    public:
        struct Range
        {
        public:
            uint32_t Slot = 0;
            ShaderStage Visibility = ShaderStage::AllGraphics;
            CD3DX12_DESCRIPTOR_RANGE Range;
        };
        struct DynamicRange
        {
        public:
            uint32_t Slot = 0;
            ShaderStage Visibility = ShaderStage::AllGraphics;
            ResourceType Type = ResourceType::None; // Note: This is used to check if an object is SRV, UAV or CBV
        };
    public:
        // Constructors & Destructor
        Dx12BindingLayout(const Device& device, const BindingLayoutSpecification& specs);
        Dx12BindingLayout(const Device& device, const BindlessLayoutSpecification& specs);
        ~Dx12BindingLayout();

        // Methods
        const BindingLayoutItem& GetItem(uint32_t slot) const;

        // Getters
        inline bool IsBindless() const { return std::holds_alternative<BindlessLayoutSpecification>(m_Specification); }

        // Internal getters
        const BindingLayoutSpecification& GetBindingSpecification() const { OB_ASSERT(!IsBindless(), "[Dx12BindingLayout] Getting BindingLayout but the layout is bindless."); return std::get<BindingLayoutSpecification>(m_Specification); }
        const BindlessLayoutSpecification& GetBindlessSpecification() const { OB_ASSERT(IsBindless(), "[Dx12BindingLayout] Getting BindlessLayout but the layout isn't bindless."); return std::get<BindlessLayoutSpecification>(m_Specification); }

        uint8_t GetRegisterSpace() const;
        std::span<const BindingLayoutItem> GetBindingItems() const;

        inline const std::vector<std::pair<ResourceType, uint32_t>>& GetResourceCounts() const { return m_ResourceCounts; }
        inline uint32_t GetSlotToHeapOffset(uint32_t slot) const { return m_SlotToHeapOffset[slot]; }
        inline const std::vector<uint32_t>& GetSlotToHeapOffsets() const { return m_SlotToHeapOffset; }

        inline const std::vector<Range>& GetSRVAndUAVAndCBVRanges() const { return m_SRVAndUAVAndCBVRanges; }
        inline const std::vector<Range>& GetSamplerRanges() const { return m_SamplerRanges; }
        inline const std::vector<DynamicRange>& GetDynamicRanges() const { return m_DynamicRanges; }
    
    private:
        // Private methods
        void InitResourceCounts(std::span<const BindingLayoutItem> items);
        void CreateRanges(std::span<const BindingLayoutItem> items);

    private:
        std::variant<BindingLayoutSpecification, BindlessLayoutSpecification> m_Specification;

        std::vector<std::pair<ResourceType, uint32_t>> m_ResourceCounts = { };
        std::vector<uint32_t> m_SlotToHeapOffset = {};

        // Note: The ranges are sorted from slot lowest to highest
        std::vector<Range> m_SRVAndUAVAndCBVRanges; // TODO: Make actual ranges instead of seperate elements a seperate range
        std::vector<Range> m_SamplerRanges;
        std::vector<DynamicRange> m_DynamicRanges; // Note: We don't need a seperate sampler one, since there is no such thing as a dynamic sampler.
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12BindingSet
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12BindingSet
    {
    public:
        // Constructor & Destructor
        Dx12BindingSet(BindingSetPool& pool, const BindingSetSpecification& specs);
        ~Dx12BindingSet();

        // Methods
        void SetItem(uint32_t slot, Image& image, const ImageSubresourceSpecification& subresources, uint32_t arrayIndex);
        void SetItem(uint32_t slot, Sampler& sampler, uint32_t arrayIndex);
        void SetItem(uint32_t slot, Buffer& buffer, const BufferRange& range, uint32_t arrayIndex);

        // Getters
        inline const BindingSetSpecification& GetSpecification() const { return m_Specification; }

        // Internal getters
        const Dx12BindingSetPool& GetDx12BindingSetPool() const { return m_Pool; }

        inline DescriptorHeapIndex GetSRVAndUAVAndCBVBeginIndex() const { return m_SRVAndUAVAndCBVBeginIndex; }
        inline DescriptorHeapIndex GetSamplerBeginIndex() const { return m_SamplerBeginIndex; }

        //inline D3D12_GPU_VIRTUAL_ADDRESS GetD3D12GPUAddressForDynamicSlot(uint32_t slot) const { OB_ASSERT((m_DynamicBeginAddresses.contains(slot)), "[Dx12BindingSet] Internal error: Trying to access a slot which wasn't created to be dynamic."); return m_DynamicBeginAddresses.at(slot); }
        
        inline D3D12_GPU_VIRTUAL_ADDRESS GetD3D12GPUAddressForDynamicSlot(uint32_t slot) const 
        { 
            OB_ASSERT((m_DynamicBeginAddresses.contains(slot)), "[Dx12BindingSet] Internal error: Trying to access a slot which wasn't created to be dynamic."); 
            return m_DynamicBeginAddresses.at(slot); 
        }

    private:
        Dx12BindingSetPool& m_Pool;
        BindingSetSpecification m_Specification;

        DescriptorHeapIndex m_SRVAndUAVAndCBVBeginIndex = 0;
        DescriptorHeapIndex m_SamplerBeginIndex = 0;

        // Note: Currently slot to address, ordered. // FUTURE TODO: ...
        std::map<uint32_t, D3D12_GPU_VIRTUAL_ADDRESS> m_DynamicBeginAddresses;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12BindingSetPool
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12BindingSetPool
    {
    public:
        // Constructor & Destructor
        Dx12BindingSetPool(const Device& device, const BindingSetPoolSpecification& specs);
        ~Dx12BindingSetPool();

        // Getters
        inline const BindingSetPoolSpecification& GetSpecification() const { return m_Specification; }

        // Internal methods
        void GetNextSetIndices(DescriptorHeapIndex& outSRVAndUAVBeginIndex, DescriptorHeapIndex& outSamplerBeginIndex);

        // Internal getters
        inline const Dx12Device& GetDx12Device() const { return m_Device; }

    private:
        const Dx12Device& m_Device;
        BindingSetPoolSpecification m_Specification;

        uint32_t m_SRVAndUAVAndCBVCountPerSet = 0;
        uint32_t m_SamplerCountPerSet = 0;

        uint32_t m_CurrentSet = 0;

        DescriptorHeapIndex m_SRVAndUAVAndCBVBeginIndex = 0;
        DescriptorHeapIndex m_SamplerBeginIndex = 0;
    };
#endif

}