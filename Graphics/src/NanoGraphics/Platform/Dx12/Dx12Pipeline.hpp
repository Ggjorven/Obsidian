#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/PipelineSpec.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"

#include <array>
#include <limits>
#include <numeric>

namespace Nano::Graphics
{
    class Device;
}

namespace Nano::Graphics::Internal
{

    class Dx12Device;
    class Dx12GraphicsPipeline;
    class Dx12ComputePipeline;

#if defined(NG_API_DX12)
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
            //                    Slot      Index
            std::vector<std::pair<uint32_t, uint16_t>> SRVAndUAVAndCBVIndices = { };
            std::vector<std::pair<uint32_t, uint16_t>> SamplerIndices = { };
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

        const std::vector<std::pair<uint32_t, uint16_t>>& GetSRVAndUAVAndCBVRootIndices(uint8_t registerSpace) const;
        const std::vector<std::pair<uint32_t, uint16_t>>& GetSamplerRootIndices(uint8_t registerSpace) const;

    private:
        GraphicsPipelineSpecification m_Specification;

        DxPtr<ID3D12RootSignature> m_RootSignature = nullptr;
        DxPtr<ID3D12PipelineState> m_PipelineState = nullptr;

        // Note: The index is the Set ID / Register space
        std::pair<uint32_t, uint16_t> m_PushConstantsIndex = { 0, RootParameterIndices::Invalid };
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

        const std::vector<std::pair<uint32_t, uint16_t>>& GetSRVAndUAVAndCBVRootIndices(uint8_t registerSpace) const;
        const std::vector<std::pair<uint32_t, uint16_t>>& GetSamplerRootIndices(uint8_t registerSpace) const;

    private:
        ComputePipelineSpecification m_Specification;

        DxPtr<ID3D12RootSignature> m_RootSignature = nullptr;
        DxPtr<ID3D12PipelineState> m_PipelineState = nullptr;

        // Note: The index is the Set ID / Register space
        std::pair<uint32_t, uint16_t> m_PushConstantsIndex = { 0, RootParameterIndices::Invalid };
        std::array<RootParameterIndices, GraphicsPipelineSpecification::MaxBindings> m_RootParameterIndices = {};

        friend class Dx12Device;
    };
#endif

}