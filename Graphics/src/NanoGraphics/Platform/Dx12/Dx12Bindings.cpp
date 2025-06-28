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

        std::span<const BindingLayoutItem> items = std::span<const BindingLayoutItem>(specs.Bindings.begin(), specs.Bindings.end());

        InitResourceCounts(items);
        CreateRootSignature(device, items);
    }

    Dx12BindingLayout::Dx12BindingLayout(const Device& device, const BindlessLayoutSpecification& specs)
        : m_Specification(specs)
    {
        (void)device;

        std::span<const BindingLayoutItem> items = std::span<const BindingLayoutItem>(specs.Bindings.begin(), specs.Bindings.end());

        InitResourceCounts(items);
        NG_ASSERT(false, "[Dx12BindingLayout] Bindless on dx12 is not properly supported.");
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

    void Dx12BindingLayout::CreateRootSignature(const Device& device, std::span<const BindingLayoutItem> items)
    {
        NG_ASSERT(!IsBindless(), "[Dx12BindingLayout] Root parameters can currently only be made for bindingsets instead of bindless.");

        const Dx12Device& dxDevice = *api_cast<const Dx12Device*>(&device);

        std::vector<CD3DX12_DESCRIPTOR_RANGE> srvRanges;
        ShaderStage srvStage = ShaderStage::None;
        srvRanges.reserve(items.size());

        std::vector<CD3DX12_DESCRIPTOR_RANGE> uavRanges;
        ShaderStage uavStage = ShaderStage::None;
        uavRanges.reserve(items.size());

        std::vector<CD3DX12_DESCRIPTOR_RANGE> cbvRanges;
        ShaderStage cbvStage = ShaderStage::None;
        cbvRanges.reserve(items.size());

        std::vector<CD3DX12_DESCRIPTOR_RANGE> samplerRanges;
        ShaderStage samplerStage = ShaderStage::None;
        samplerRanges.reserve(items.size());

        // Create ranges
        {
            const BindingLayoutSpecification& specs = std::get<BindingLayoutSpecification>(m_Specification);

            for (const auto& item : items)
            {
                switch (item.Type)
                {
                case ResourceType::Image:
                case ResourceType::StorageBuffer:
                {
                    CD3DX12_DESCRIPTOR_RANGE& range = srvRanges.emplace_back();
                    range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, item.Slot, specs.RegisterSpace); // 1 descriptor, register t0, space 2

                    if ((srvStage != ShaderStage::None) && (srvStage != item.Visibility))
                        srvStage = ShaderStage::AllGraphics;
                    else
                        srvStage = item.Visibility;

                    break;
                }

                case ResourceType::ImageUnordered:
                case ResourceType::StorageBufferUnordered:
                {
                    CD3DX12_DESCRIPTOR_RANGE& range = uavRanges.emplace_back();
                    range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, item.Slot, specs.RegisterSpace); // 1 descriptor, register t0, space 2

                    if ((uavStage != ShaderStage::None) && (uavStage != item.Visibility))
                        uavStage = ShaderStage::AllGraphics;
                    else
                        uavStage = item.Visibility;

                    break;
                }

                case ResourceType::UniformBuffer:
                {
                    CD3DX12_DESCRIPTOR_RANGE& range = cbvRanges.emplace_back();
                    range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, item.Slot, specs.RegisterSpace); // 1 descriptor, register t0, space 2

                    if ((cbvStage != ShaderStage::None) && (cbvStage != item.Visibility))
                        cbvStage = ShaderStage::AllGraphics;
                    else
                        cbvStage = item.Visibility;

                    break;
                }

                case ResourceType::Sampler:
                {
                    CD3DX12_DESCRIPTOR_RANGE& range = samplerRanges.emplace_back();
                    range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, item.Slot, specs.RegisterSpace); // 1 descriptor, register t0, space 2

                    if ((samplerStage != ShaderStage::None) && (samplerStage != item.Visibility))
                        samplerStage = ShaderStage::AllGraphics;
                    else
                        samplerStage = item.Visibility;

                    break;
                }

                case ResourceType::DynamicStorageBuffer:
                case ResourceType::DynamicUniformBuffer:
                    NG_ASSERT(false, "[Dx12BindingLayout] Dynamic buffer are currently not supported in Dx12."); // FUTURE TODO: ...
                    break;

                case ResourceType::PushConstants:
                    // Note: There is no Dx12 equivalent // FUTURE TODO: ...
                    break;
                }
            }
        }

        // Create root params
        std::array<CD3DX12_ROOT_PARAMETER, 4> parameters = {};
        CD3DX12_ROOT_PARAMETER& descriptorSRVRootParameter = parameters[0];
        CD3DX12_ROOT_PARAMETER& descriptorUAVRootParameter = parameters[1];
        CD3DX12_ROOT_PARAMETER& descriptorCBVRootParameter = parameters[2];
        CD3DX12_ROOT_PARAMETER& descriptorSamplerRootParameter = parameters[3];
        {
            descriptorSRVRootParameter.InitAsDescriptorTable(static_cast<UINT>(srvRanges.size()), srvRanges.data(), ShaderStageToD3D12ShaderVisibility(srvStage));
            descriptorUAVRootParameter.InitAsDescriptorTable(static_cast<UINT>(uavRanges.size()), uavRanges.data(), ShaderStageToD3D12ShaderVisibility(uavStage));
            descriptorCBVRootParameter.InitAsDescriptorTable(static_cast<UINT>(cbvRanges.size()), cbvRanges.data(), ShaderStageToD3D12ShaderVisibility(cbvStage));
            descriptorSamplerRootParameter.InitAsDescriptorTable(static_cast<UINT>(samplerRanges.size()), samplerRanges.data(), ShaderStageToD3D12ShaderVisibility(samplerStage));
        }

        // Create root signature
        {
            // Create root signature with these parameters
            CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
            rootSigDesc.Init(static_cast<UINT>(parameters.size()), parameters.data());

            DxPtr<ID3DBlob> serializedRootSig;
            DxPtr<ID3DBlob> errorBlob;
            D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);
            
            dxDevice.GetContext().GetD3D12Device()->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
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
        (void)arrayIndex; // TODO: Maybe use somehow or remove.

        Dx12BindingLayout& dxLayout = *api_cast<Dx12BindingLayout*>(m_Pool.GetSpecification().Layout);
        const auto& item = dxLayout.GetItem(slot);

        NG_ASSERT(((item.Type == ResourceType::Image) || (item.Type == ResourceType::ImageUnordered)), "[Dx12BindingSet] When uploading an image the ResourceType must be Image or ImageUnordered.");

        Dx12Image& dxImage = *api_cast<Dx12Image*>(&image);
        DescriptorHeapIndex index = m_SRVAndUAVBeginIndex + slot;

        switch (item.Type)
        {
        case ResourceType::Image:
            m_Pool.GetDx12Device().GetResources().GetSRVAndUAVHeap().CreateSRV(index, image.GetSpecification(), subresources, dxImage.GetD3D12Resource().Get());
            break;

        case ResourceType::ImageUnordered:
            m_Pool.GetDx12Device().GetResources().GetSRVAndUAVHeap().CreateUAV(index, image.GetSpecification(), subresources, dxImage.GetD3D12Resource().Get());
            break;

        default:
            NG_UNREACHABLE();
            break;
        }
    }

    void Dx12BindingSet::SetItem(uint32_t slot, Sampler& sampler, uint32_t arrayIndex)
    {
        (void)arrayIndex; // TODO: Maybe use somehow or remove.

        Dx12BindingLayout& dxLayout = *api_cast<Dx12BindingLayout*>(m_Pool.GetSpecification().Layout);
        const auto& item = dxLayout.GetItem(slot);

        NG_ASSERT((item.Type == ResourceType::Sampler), "[Dx12BindingSet] When uploading a sampler the ResourceType must be Sampler.");

        //Dx12Sampler& dxSampler = *api_cast<Dx12Sampler*>(&sampler);
        DescriptorHeapIndex index = m_SamplerBeginIndex + slot;

        m_Pool.GetDx12Device().GetResources().GetSamplerHeap().CreateSampler(index, sampler.GetSpecification());
    }

    void Dx12BindingSet::SetItem(uint32_t slot, Buffer& buffer, const BufferRange& range, uint32_t arrayIndex)
    {
        (void)arrayIndex; // TODO: Maybe use somehow or remove.

        Dx12BindingLayout& dxLayout = *api_cast<Dx12BindingLayout*>(m_Pool.GetSpecification().Layout);
        const auto& item = dxLayout.GetItem(slot);

        NG_ASSERT(((item.Type == ResourceType::StorageBuffer) || (item.Type == ResourceType::StorageBufferUnordered) || (item.Type == ResourceType::DynamicStorageBuffer) || (item.Type == ResourceType::UniformBuffer) || (item.Type == ResourceType::DynamicUniformBuffer)), "[Dx12BindingSet] When uploading a buffer the ResourceType must be StorageBuffer, StorageBufferUnordered, DynamicStorageBuffer, UniformBuffer or DynamicUniformBuffer.");

        Dx12Buffer& dxBuffer = *api_cast<Dx12Buffer*>(&buffer);
        DescriptorHeapIndex index = m_SRVAndUAVBeginIndex + slot;

        switch (item.Type)
        {
        case ResourceType::StorageBuffer:
            m_Pool.GetDx12Device().GetResources().GetSRVAndUAVHeap().CreateSRV(index, buffer.GetSpecification(), range, dxBuffer.GetD3D12Resource().Get());
            break;

        case ResourceType::StorageBufferUnordered:
            m_Pool.GetDx12Device().GetResources().GetSRVAndUAVHeap().CreateUAV(index, buffer.GetSpecification(), range, dxBuffer.GetD3D12Resource().Get());
            break;

        case ResourceType::UniformBuffer:
            m_Pool.GetDx12Device().GetResources().GetSRVAndUAVHeap().CreateCBV(index, buffer.GetSpecification(), dxBuffer.GetD3D12Resource().Get());
            break;

        case ResourceType::DynamicStorageBuffer:
        case ResourceType::DynamicUniformBuffer:
            NG_ASSERT(false, "[Dx12BindingSet] Dynamic buffers are not implemented on dx12.");
            break;

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
                // FUTURE TODO: Dx12 types
                switch (type)
                {
                case ResourceType::Image:
                case ResourceType::StorageBuffer:
                case ResourceType::UniformBuffer:
                // Purposeful fallthrough
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