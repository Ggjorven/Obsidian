#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Maths/Structs.hpp"

#include "Obsidian/Renderer/ResourceSpec.hpp"
#include "Obsidian/Renderer/PipelineSpec.hpp"

#include "Obsidian/Platform/Dx12/Dx12.hpp"

#include <array>
#include <limits>
#include <numeric>

namespace Obsidian
{
    class Device;
}

namespace Obsidian::Internal
{

    class Dx12Device;
    class Dx12GraphicsPipeline;
    class Dx12ComputePipeline;

#if defined(OB_API_DX12)
    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12GraphicsPipeline
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12GraphicsPipeline
    {
    public:
        struct RootParameterIndices
        {
        public:
            inline constexpr static uint16_t Invalid = std::numeric_limits<uint16_t>::max();
        public:
            struct IndexType
            {
            public:
                uint32_t Slot = 0;
                uint16_t Index = Invalid;
            };
            struct DynamicIndexType
            {
            public:
                uint32_t Slot = 0;
                uint16_t Index = Invalid;
                ResourceType Type = ResourceType::None;
            };
        public:
            std::vector<IndexType> SRVAndUAVAndCBVIndices = { };
            std::vector<IndexType> SamplerIndices = { };
            std::vector<DynamicIndexType> DynamicIndices = { };
        };
    public:
        // Constructor & Destructor
        Dx12GraphicsPipeline(const Device& device, const GraphicsPipelineSpecification& specs);
        ~Dx12GraphicsPipeline();

        // Getters
        inline const GraphicsPipelineSpecification& GetSpecification() const { return m_Specification; }

        // Internal getters
        inline DxPtr<ID3D12RootSignature> GetD3D12RootSignature() const { return m_RootSignature; }
        inline DxPtr<ID3D12PipelineState> GetD3D12PipelineState() const { return m_PipelineState; }

        inline const RootParameterIndices::IndexType& GetPushConstantsRootIndex() const { return m_PushConstantsIndex; }
        
        const std::vector<RootParameterIndices::IndexType>& GetSRVAndUAVAndCBVRootIndices(uint8_t registerSpace) const;
        const std::vector<RootParameterIndices::IndexType>& GetSamplerRootIndices(uint8_t registerSpace) const;
        const std::vector<RootParameterIndices::DynamicIndexType>& GetDynamicRootIndices(uint8_t registerSpace) const;

    private:
        GraphicsPipelineSpecification m_Specification;

        DxPtr<ID3D12RootSignature> m_RootSignature = nullptr;
        DxPtr<ID3D12PipelineState> m_PipelineState = nullptr;

        RootParameterIndices::IndexType m_PushConstantsIndex = { 0, RootParameterIndices::Invalid };
        std::array<RootParameterIndices, GraphicsPipelineSpecification::MaxBindings> m_RootParameterIndices = {};

        friend class Dx12Device;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12ComputePipeline
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12ComputePipeline
    {
    public:
        using RootParameterIndices = Dx12GraphicsPipeline::RootParameterIndices;
    public:
        // Constructors & Destructor
        Dx12ComputePipeline(const Device& device, const ComputePipelineSpecification& specs);
        ~Dx12ComputePipeline();

        // Getters
        inline const ComputePipelineSpecification& GetSpecification() const { return m_Specification; }

        // Internal getters
        inline DxPtr<ID3D12RootSignature> GetD3D12RootSignature() const { return m_RootSignature; }
        inline DxPtr<ID3D12PipelineState> GetD3D12PipelineState() const { return m_PipelineState; }

        inline const RootParameterIndices::IndexType& GetPushConstantsRootIndex() const { return m_PushConstantsIndex; }

        const std::vector<RootParameterIndices::IndexType>& GetSRVAndUAVAndCBVRootIndices(uint8_t registerSpace) const;
        const std::vector<RootParameterIndices::IndexType>& GetSamplerRootIndices(uint8_t registerSpace) const;
        const std::vector<RootParameterIndices::DynamicIndexType>& GetDynamicRootIndices(uint8_t registerSpace) const;

    private:
        ComputePipelineSpecification m_Specification;

        DxPtr<ID3D12RootSignature> m_RootSignature = nullptr;
        DxPtr<ID3D12PipelineState> m_PipelineState = nullptr;

        RootParameterIndices::IndexType m_PushConstantsIndex = { 0, RootParameterIndices::Invalid };
        std::array<RootParameterIndices, GraphicsPipelineSpecification::MaxBindings> m_RootParameterIndices = {};

        friend class Dx12Device;
    };
#endif

}