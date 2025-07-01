#include "ngpch.h"
#include "Dx12Pipeline.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12Device.hpp"

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12GraphicsPipeline::Dx12GraphicsPipeline(const Device& device, const GraphicsPipelineSpecification& specs)
        : m_Specification(specs)
    {
        const Dx12Device& dxDevice = *api_cast<const Dx12Device*>(&device);

        // Create root signature
        {
            std::vector<CD3DX12_ROOT_PARAMETER> parameters;
            parameters.reserve(specs.BindingLayouts.size() * Dx12BindingLayout::ParameterCount);

            // Create one big list of parameters
            for (auto& layout : specs.BindingLayouts)
            {
                Dx12BindingLayout& dxLayout = *api_cast<Dx12BindingLayout*>(layout);

                const auto& params = dxLayout.GetD3D12RootParameters();
                if (!params.empty())
                    parameters.insert(parameters.end(), params.begin(), params.end());
            }

            // Create root signature with these parameters
            CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc = {};
            
            if (!parameters.empty())
                rootSigDesc.Init(static_cast<UINT>(parameters.size()), parameters.data());
            
            rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

            DxPtr<ID3DBlob> serializedRootSig;
            DxPtr<ID3DBlob> errorBlob;
            DX_VERIFY(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob));

            DX_VERIFY(dxDevice.GetContext().GetD3D12Device()->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));
        }

        // Create pipeline state
        {
            NG_ASSERT(m_Specification.Pass, "[Dx12GraphicsPipeline] No proper renderpass was passed in.");
            NG_ASSERT(m_Specification.Input, "[Dx12GraphicsPipeline] No proper input layout was passed in.");

            auto& blendState = m_Specification.RenderingState.Blend;
            auto& depthStencilState = m_Specification.RenderingState.DepthStencil;
            auto& rasterState = m_Specification.RenderingState.Raster;

            D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
            desc.pRootSignature = m_RootSignature.Get();
            
            // Shader
            if (m_Specification.VertexShader) desc.VS = api_cast<Dx12Shader*>(m_Specification.VertexShader)->GetD3D12ShaderByteCode();
            if (m_Specification.FragmentShader) desc.PS = api_cast<Dx12Shader*>(m_Specification.FragmentShader)->GetD3D12ShaderByteCode();
            if (m_Specification.TesselationEvaluationShader) desc.DS = api_cast<Dx12Shader*>(m_Specification.TesselationEvaluationShader)->GetD3D12ShaderByteCode();
            if (m_Specification.TesselationControlShader) desc.HS = api_cast<Dx12Shader*>(m_Specification.TesselationControlShader)->GetD3D12ShaderByteCode();
            if (m_Specification.GeometryShader) desc.GS = api_cast<Dx12Shader*>(m_Specification.GeometryShader)->GetD3D12ShaderByteCode();

            // Blend
            desc.BlendState.AlphaToCoverageEnable = blendState.AlphaToCoverageEnable;
            desc.BlendState.IndependentBlendEnable = true;

            desc.BlendState.RenderTarget[0].BlendEnable = blendState.Target.BlendEnable ? TRUE : FALSE;
            desc.BlendState.RenderTarget[0].SrcBlend = BlendFactorToD3D12Blend(blendState.Target.SrcBlend);
            desc.BlendState.RenderTarget[0].DestBlend = BlendFactorToD3D12Blend(blendState.Target.DstBlend);
            desc.BlendState.RenderTarget[0].BlendOp = BlendOperationToD3D12BlendOp(blendState.Target.BlendOp);
            desc.BlendState.RenderTarget[0].SrcBlendAlpha = BlendFactorToD3D12Blend(blendState.Target.SrcBlendAlpha);
            desc.BlendState.RenderTarget[0].DestBlendAlpha = BlendFactorToD3D12Blend(blendState.Target.DstBlendAlpha);
            desc.BlendState.RenderTarget[0].BlendOpAlpha = BlendOperationToD3D12BlendOp(blendState.Target.BlendOpAlpha);
            desc.BlendState.RenderTarget[0].RenderTargetWriteMask = static_cast<UINT8>(ColourMaskToD3D12ColourWriteEnable(blendState.Target.ColourWriteMask));

            // Depth stencil
            desc.DepthStencilState.DepthEnable = depthStencilState.DepthTestEnable ? TRUE : FALSE;
            desc.DepthStencilState.DepthWriteMask = depthStencilState.DepthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
            desc.DepthStencilState.DepthFunc = ComparisonFuncToD3D12ComparisonFunc(depthStencilState.DepthFunc);
            desc.DepthStencilState.StencilEnable = depthStencilState.StencilEnable ? TRUE : FALSE;
            desc.DepthStencilState.StencilReadMask = depthStencilState.StencilReadMask;
            desc.DepthStencilState.StencilWriteMask = depthStencilState.StencilWriteMask;
            desc.DepthStencilState.FrontFace.StencilFailOp = StencilOperationToD3D12StencilOp(depthStencilState.FrontFaceStencil.FailOp);
            desc.DepthStencilState.FrontFace.StencilDepthFailOp = StencilOperationToD3D12StencilOp(depthStencilState.FrontFaceStencil.DepthFailOp);
            desc.DepthStencilState.FrontFace.StencilPassOp = StencilOperationToD3D12StencilOp(depthStencilState.FrontFaceStencil.PassOp);
            desc.DepthStencilState.FrontFace.StencilFunc = ComparisonFuncToD3D12ComparisonFunc(depthStencilState.FrontFaceStencil.StencilFunc);
            desc.DepthStencilState.BackFace.StencilFailOp = StencilOperationToD3D12StencilOp(depthStencilState.BackFaceStencil.FailOp);
            desc.DepthStencilState.BackFace.StencilDepthFailOp = StencilOperationToD3D12StencilOp(depthStencilState.BackFaceStencil.DepthFailOp);
            desc.DepthStencilState.BackFace.StencilPassOp = StencilOperationToD3D12StencilOp(depthStencilState.BackFaceStencil.PassOp);
            desc.DepthStencilState.BackFace.StencilFunc = ComparisonFuncToD3D12ComparisonFunc(depthStencilState.BackFaceStencil.StencilFunc);

            if ((depthStencilState.DepthTestEnable || depthStencilState.StencilEnable) && (m_Specification.Pass->GetFramebuffer(0).GetSpecification().DepthAttachment.IsValid()) && (m_Specification.Pass->GetFramebuffer(0).GetSpecification().DepthAttachment.ImagePtr->GetSpecification().ImageFormat == Format::Unknown))
            {
                desc.DepthStencilState.DepthEnable = FALSE;
                desc.DepthStencilState.StencilEnable = FALSE;

                desc.DSVFormat = FormatToFormatMapping(m_Specification.Pass->GetFramebuffer(0).GetSpecification().DepthAttachment.ImagePtr->GetSpecification().ImageFormat).RTVFormat;

                dxDevice.GetContext().Warn("[Dx12GraphicsPipeline] DepthEnable or StencilEnable is true, but no depth target is set for the renderpass' framebuffer.");
            }

            // Rasterizer state
            desc.RasterizerState.FillMode = RasterFillModeTOD3D12FillMode(rasterState.FillMode);
            desc.RasterizerState.CullMode = RasterCullingModeTOD3D12CullMode(rasterState.CullingMode);
            desc.RasterizerState.FrontCounterClockwise = rasterState.FrontCounterClockwise ? TRUE : FALSE;
            desc.RasterizerState.DepthBias = rasterState.DepthBias;
            desc.RasterizerState.DepthBiasClamp = rasterState.DepthBiasClamp;
            desc.RasterizerState.SlopeScaledDepthBias = rasterState.SlopeScaledDepthBias;
            desc.RasterizerState.DepthClipEnable = rasterState.DepthClipEnable ? TRUE : FALSE;
            desc.RasterizerState.MultisampleEnable = rasterState.MultisampleEnable ? TRUE : FALSE;
            desc.RasterizerState.AntialiasedLineEnable = rasterState.AntialiasedLineEnable ? TRUE : FALSE;
            
            desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            desc.RasterizerState.ForcedSampleCount = 0;

            desc.PrimitiveTopologyType = PrimitiveTypeToD3D12PrimitiveTopology(m_Specification.Primitive);
            desc.SampleDesc.Count = m_Specification.Pass->GetFramebuffer(0).GetSpecification().ColourAttachment.ImagePtr->GetSpecification().SampleCount;
            desc.SampleDesc.Quality = m_Specification.Pass->GetFramebuffer(0).GetSpecification().ColourAttachment.ImagePtr->GetSpecification().SampleQuality;

            desc.RTVFormats[0] = FormatToFormatMapping(m_Specification.Pass->GetFramebuffer(0).GetSpecification().ColourAttachment.ImagePtr->GetSpecification().ImageFormat).RTVFormat;

            Dx12InputLayout& dxInputLayout = *api_cast<Dx12InputLayout*>(m_Specification.Input);
            desc.InputLayout.NumElements = static_cast<uint32_t>(dxInputLayout.GetInputElements().size());
            desc.InputLayout.pInputElementDescs = dxInputLayout.GetInputElements().data();

            desc.NumRenderTargets = 1;
            desc.SampleMask = ~0u;

            DX_VERIFY(dxDevice.GetContext().GetD3D12Device()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_PipelineState)));
        }

        if constexpr (Information::Validation)
        {
            if (!m_Specification.DebugName.empty())
            {
                dxDevice.GetContext().SetDebugName(m_PipelineState.Get(), m_Specification.DebugName);
                dxDevice.GetContext().SetDebugName(m_RootSignature.Get(), std::format("RootSignature for: {0}", m_Specification.DebugName));
            }
        }
    }

    Dx12GraphicsPipeline::~Dx12GraphicsPipeline()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12ComputePipeline::Dx12ComputePipeline(const Device& device, const ComputePipelineSpecification& specs)
        : m_Specification(specs)
    {
        const Dx12Device& dxDevice = *api_cast<const Dx12Device*>(&device);

        // TODO: ...
    }

    Dx12ComputePipeline::~Dx12ComputePipeline()
    {
    }

}