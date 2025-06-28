#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/BindingsSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Descriptors.hpp"

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

    class Dx12Device;
    class Dx12BindingLayout;
    class Dx12BindingSet;
    class Dx12BindingSetPool;

#if defined(NG_API_DX12)
    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12BindingLayout
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12BindingLayout
    {
    public:
        inline constexpr static size_t ParameterCount = 4;
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
        std::span<const BindingLayoutItem> GetBindingItems() const;

        inline const std::vector<std::pair<ResourceType, uint32_t>>& GetResourceCounts() const { return m_ResourceCounts; }

        inline const std::array<CD3DX12_ROOT_PARAMETER, ParameterCount>& GetD3D12RootParameters() const { return m_Parameters; }

    private:
        // Private methods
        void InitResourceCounts(std::span<const BindingLayoutItem> items);
        void CreateRootParameters(const Device& device, std::span<const BindingLayoutItem> items);

    private:
        std::variant<BindingLayoutSpecification, BindlessLayoutSpecification> m_Specification;

        std::vector<std::pair<ResourceType, uint32_t>> m_ResourceCounts = { };

        std::vector<CD3DX12_DESCRIPTOR_RANGE> m_SRVRanges;
        std::vector<CD3DX12_DESCRIPTOR_RANGE> m_UAVRanges;
        std::vector<CD3DX12_DESCRIPTOR_RANGE> m_CBVRanges;
        std::vector<CD3DX12_DESCRIPTOR_RANGE> m_SamplerRanges;

        std::array<CD3DX12_ROOT_PARAMETER, ParameterCount> m_Parameters = {};

        // Note: For ease of use, will be optimized away by compiler (I hope)
        CD3DX12_ROOT_PARAMETER& m_DescriptorSRVRootParameter = m_Parameters[0];
        CD3DX12_ROOT_PARAMETER& m_DescriptorUAVRootParameter = m_Parameters[1];
        CD3DX12_ROOT_PARAMETER& m_DescriptorCBVRootParameter = m_Parameters[2];
        CD3DX12_ROOT_PARAMETER& m_DescriptorSamplerRootParameter = m_Parameters[3];
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

    private:
        Dx12BindingSetPool& m_Pool;
        BindingSetSpecification m_Specification;

        DescriptorHeapIndex m_SRVAndUAVBeginIndex = 0;
        DescriptorHeapIndex m_SamplerBeginIndex = 0;
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

        uint32_t m_SRVAndUAVCountPerSet = 0;
        uint32_t m_SamplerCountPerSet = 0;

        uint32_t m_CurrentSet = 0;

        DescriptorHeapIndex m_SRVAndUAVBeginIndex = 0;
        DescriptorHeapIndex m_SamplerBeginIndex = 0;
    };
#endif

}