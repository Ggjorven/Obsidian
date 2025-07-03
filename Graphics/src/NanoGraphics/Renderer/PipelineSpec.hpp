#pragma once

#include "NanoGraphics/Renderer/BindingsSpec.hpp"

#include <Nano/Nano.hpp>

#include <cstdint>
#include <string>

namespace Nano::Graphics
{

    class Shader;
    class Renderpass;
    class InputLayout;

    ////////////////////////////////////////////////////////////////////////////////////
    // Flags
    ////////////////////////////////////////////////////////////////////////////////////
    enum class PrimitiveType : uint8_t
    {
        PointList = 0,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        TriangleFan,
        TriangleListWithAdjacency,
        TriangleStripWithAdjacency,
        //PatchList
    };

    enum class BlendFactor : uint8_t
    {
        Zero = 0, One,
        SrcColour, InvSrcColour,
        SrcAlpha, InvSrcAlpha,
        DstAlpha, InvDstAlpha,
        DstColour, InvDstColour,
        SrcAlphaSaturate, 
        ConstantColour, InvConstantColour,
        Src1Colour, InvSrc1Colour,
        Src1Alpha, InvSrc1Alpha,

        // Vulkan names
        OneMinusSrcColour = InvSrcColour,
        OneMinusSrcAlpha = InvSrcAlpha,
        OneMinusDstAlpha = InvDstAlpha,
        OneMinusDstColour = InvDstColour,
        OneMinusConstantColour = InvConstantColour,
        OneMinusSrc1Colour = InvSrc1Colour,
        OneMinusSrc1Alpha = InvSrc1Alpha,
    };

    enum class BlendOperation : uint8_t
    {
        Add = 0,
        Subtract, ReverseSubtract,
        Min, Max
    };

    enum class ColourMask : uint8_t
    {
        None = 0,

        Red = 1 << 0,
        Green = 1 << 1,
        Blue = 1 << 2,
        Alpha = 1 << 3,

        All = Red | Green | Blue | Alpha
    };

    NANO_DEFINE_BITWISE(ColourMask)

    enum class RasterFillMode : uint8_t // Note: Corresponds to Vulkan's PolygonMode
    {
        Solid = 0,
        Wireframe,

        // Vulkan names
        Fill = Solid,
        Line = Wireframe
    };

    enum class RasterCullingMode : uint8_t
    {
        None = 0,
        Back,
        Front,
    };

    enum class StencilOperation : uint8_t
    {
        Keep = 0,
        Zero,
        Replace,
        IncrementAndClamp, DecrementAndClamp,
        Invert,
        IncrementAndWrap, DecrementAndWrap
    };

    enum class ComparisonFunc : uint8_t
    {
        Never = 0,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Structs
    ////////////////////////////////////////////////////////////////////////////////////
    struct BlendState
    {
    public:
        struct RenderTarget
        {
        public:
            bool BlendEnable = false;
            BlendFactor SrcBlend = BlendFactor::One;
            BlendFactor DstBlend = BlendFactor::Zero;
            BlendOperation BlendOp = BlendOperation::Add;
            BlendFactor SrcBlendAlpha = BlendFactor::One;
            BlendFactor DstBlendAlpha = BlendFactor::Zero;
            BlendOperation BlendOpAlpha = BlendOperation::Add;
            ColourMask ColourWriteMask = ColourMask::All;

        public:
            // Setters
            inline constexpr RenderTarget& SetBlendEnable(bool enabled) { BlendEnable = enabled; return *this; }
            inline constexpr RenderTarget& SetSrcBlend(BlendFactor factor) { SrcBlend = factor; return *this; }
            inline constexpr RenderTarget& SetDstBlend(BlendFactor factor) { DstBlend = factor; return *this; }
            inline constexpr RenderTarget& SetBlendOperation(BlendOperation operation) { BlendOp = operation; return *this; }
            inline constexpr RenderTarget& SetSrcBlendAlpha(BlendFactor factor) { SrcBlendAlpha = factor; return *this; }
            inline constexpr RenderTarget& SetDstBlendAlpha(BlendFactor factor) { DstBlendAlpha = factor; return *this; }
            inline constexpr RenderTarget& SetBlendOpAlpha(BlendOperation operation) { BlendOpAlpha = operation; return *this; }
            inline constexpr RenderTarget& SetColourWriteMask(ColourMask mask) { ColourWriteMask = mask; return *this; }
        };
    public:
        RenderTarget Target; // Note: At this moment we only support 1 Rendertarget
        bool AlphaToCoverageEnable = false;

    public:
        // Setters
        inline constexpr BlendState& SetRenderTarget(const RenderTarget& target) { Target = target; return *this; }
        inline constexpr BlendState& SetAlphaToCoverageEnable(bool enabled) { AlphaToCoverageEnable = enabled; return *this; }
    };

    struct RasterState
    {
    public:
        RasterFillMode FillMode = RasterFillMode::Solid;
        RasterCullingMode CullingMode = RasterCullingMode::Back;

        bool FrontCounterClockwise = false;
        bool DepthClipEnable = false;
        bool ScissorEnable = false;
        bool MultisampleEnable = false;
        bool AntialiasedLineEnable = false;
        int DepthBias = 0;
        float DepthBiasClamp = 0.f;
        float SlopeScaledDepthBias = 0.f;

    public:
        // Setters
        inline constexpr RasterState& SetFillMode(RasterFillMode mode) { FillMode = mode; return *this; }
        inline constexpr RasterState& SetCullingMode(RasterCullingMode mode) { CullingMode = mode; return *this; }
        inline constexpr RasterState& SetFrontCounterClockwise(bool enabled) { FrontCounterClockwise = enabled; return *this; }
        inline constexpr RasterState& SetDepthClipEnable(bool enabled) { DepthClipEnable = enabled; return *this; }
        inline constexpr RasterState& SetScissorEnable(bool enabled) { ScissorEnable = enabled; return *this; }
        inline constexpr RasterState& SetMultisampleEnable(bool enabled) { MultisampleEnable = enabled; return *this; }
        inline constexpr RasterState& SetAntialiasedLineEnable(bool enabled) { AntialiasedLineEnable = enabled; return *this; }
        inline constexpr RasterState& SetDepthBias(int value) { DepthBias = value; return *this; }
        inline constexpr RasterState& SetDepthBiasClamp(float value) { DepthBiasClamp = value; return *this; }
        inline constexpr RasterState& SetSlopeScaleDepthBias(float value) { SlopeScaledDepthBias = value; return *this; }
    };

    struct DepthStencilState
    {
    public:
        struct StencilOperationSpecification
        {
        public:
            StencilOperation FailOp = StencilOperation::Keep;
            StencilOperation DepthFailOp = StencilOperation::Keep;
            StencilOperation PassOp = StencilOperation::Keep;
            ComparisonFunc StencilFunc = ComparisonFunc::Always;

        public:
            // Setters
            inline constexpr StencilOperationSpecification& SetFailOp(StencilOperation operation) { FailOp = operation; return *this; }
            inline constexpr StencilOperationSpecification& SetDepthFailOp(StencilOperation operation) { DepthFailOp = operation; return *this; }
            inline constexpr StencilOperationSpecification& SetPassOp(StencilOperation operation) { PassOp = operation; return *this; }
            inline constexpr StencilOperationSpecification& SetStencilFunc(ComparisonFunc func) { StencilFunc = func; return *this; }
        };
    public:
        bool DepthTestEnable = true;
        bool DepthWriteEnable = true;
        ComparisonFunc DepthFunc = ComparisonFunc::Less;
        bool StencilEnable = false;
        uint8_t StencilReadMask = 0xff;
        uint8_t StencilWriteMask = 0xff;
        uint8_t StencilRefValue = 0;
        bool DynamicStencilRef = false;
        StencilOperationSpecification FrontFaceStencil = {};
        StencilOperationSpecification BackFaceStencil = {};

    public:
        // Setters
        inline constexpr DepthStencilState& SetDepthTestEnable(bool enabled) { DepthTestEnable = enabled; return *this; }
        inline constexpr DepthStencilState& SetDepthWriteEnable(bool enabled) { DepthWriteEnable = enabled; return *this; }
        inline constexpr DepthStencilState& SetDepthFunc(ComparisonFunc func) { DepthFunc = func; return *this; }
        inline constexpr DepthStencilState& SetStencilEnable(bool enabled) { StencilEnable = enabled; return *this; }
        inline constexpr DepthStencilState& SetStencilReadMask(uint8_t value) { StencilReadMask = value; return *this; }
        inline constexpr DepthStencilState& SetStencilWriteMask(uint8_t value) { StencilWriteMask = value; return *this; }
        inline constexpr DepthStencilState& SetStencilRefValue(uint8_t value) { StencilRefValue = value; return *this; }
        inline constexpr DepthStencilState& SetFrontFaceStencil(const StencilOperationSpecification& specs) { FrontFaceStencil = specs; return *this; }
        inline constexpr DepthStencilState& SetBackFaceStencil(const StencilOperationSpecification& specs) { BackFaceStencil = specs; return *this; }
        inline constexpr DepthStencilState& SetDynamicStencilRef(bool enabled) { DynamicStencilRef = enabled; return *this; }
    };

    struct RenderState
    {
    public:
        BlendState Blend = {};
        DepthStencilState DepthStencil = {};
        RasterState Raster = {};

    public:
        // Setters
        inline constexpr RenderState& SetBlendState(const BlendState& state) { Blend = state; return *this; }
        inline constexpr RenderState& SetDepthStencilState(const DepthStencilState& state) { DepthStencil = state; return *this; }
        inline constexpr RenderState& SetRasterState(const RasterState& state) { Raster = state; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // PushConstantSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct PushConstantSpecification
    {
    public:
        inline constexpr static uint16_t MaxSize = 128; // 128 bytes
    public:
        ShaderStage Visibility = ShaderStage::None;
        uint16_t Size = 0;

    public:
        // Setters
        inline constexpr PushConstantSpecification& SetVisibility(ShaderStage stage) { Visibility = stage; return *this; }
        inline constexpr PushConstantSpecification& SetSize(uint16_t size) { Size = size; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // GraphicsPipelineSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct GraphicsPipelineSpecification
    {
    public:
        inline constexpr static uint32_t MaxBindings = 8;
    public:
        PrimitiveType Primitive = PrimitiveType::TriangleList;
        InputLayout* Input = nullptr;

        Shader* VertexShader = nullptr;
        Shader* TesselationControlShader = nullptr;
        Shader* TesselationEvaluationShader = nullptr;
        Shader* GeometryShader = nullptr;
        Shader* FragmentShader = nullptr;
        
        RenderState RenderingState = {};
        Renderpass* Pass = nullptr;

        Nano::Memory::StaticVector<BindingLayout*, MaxBindings> BindingLayouts = {};
        PushConstantSpecification PushConstants = {};

        std::string DebugName = {};

    public:
        // Setters
        inline constexpr GraphicsPipelineSpecification& SetPrimitiveType(PrimitiveType type) { Primitive = type; return *this; }
        inline constexpr GraphicsPipelineSpecification& SetInputLayout(InputLayout& layout) { Input = &layout; return *this; }

        inline constexpr GraphicsPipelineSpecification& SetVertexShader(Shader& shader) { VertexShader = &shader; return *this; }
        inline constexpr GraphicsPipelineSpecification& SetTesselationControlShader(Shader& shader) { TesselationControlShader = &shader; return *this; }
        inline constexpr GraphicsPipelineSpecification& SetHullShader(Shader& shader) { TesselationControlShader = &shader; return *this; }
        inline constexpr GraphicsPipelineSpecification& SetTesselationEvaluationShader(Shader& shader) { TesselationEvaluationShader = &shader; return *this; }
        inline constexpr GraphicsPipelineSpecification& SetDomainShader(Shader& shader) { TesselationEvaluationShader = &shader; return *this; }
        inline constexpr GraphicsPipelineSpecification& SetGeometryShader(Shader& shader) { GeometryShader = &shader; return *this; }
        inline constexpr GraphicsPipelineSpecification& SetFragmentShader(Shader& shader) { FragmentShader = &shader; return *this; }
        inline constexpr GraphicsPipelineSpecification& SetPixelShader(Shader& shader) { FragmentShader = &shader; return *this; }

        inline constexpr GraphicsPipelineSpecification& SetRenderState(const RenderState& state) { RenderingState = state; return *this; }
        inline constexpr GraphicsPipelineSpecification& SetRenderpass(Renderpass& renderpass) { Pass = &renderpass; return *this; }

        inline GraphicsPipelineSpecification& AddBindingLayout(BindingLayout& layout) { BindingLayouts.push_back(&layout); return *this; }
        inline constexpr GraphicsPipelineSpecification& SetPushConstants(const PushConstantSpecification& pushConstants) { PushConstants = pushConstants; return *this; }

        inline GraphicsPipelineSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // ComputePipelineSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct ComputePipelineSpecification
    {
    public:
        inline constexpr static uint32_t MaxBindings = GraphicsPipelineSpecification::MaxBindings;
    public:
        Shader* ComputeShader;

        Nano::Memory::StaticVector<BindingLayout*, MaxBindings> BindingLayouts = {};
        PushConstantSpecification PushConstants = {};

        std::string DebugName = {};

    private:
        // Setters
        inline constexpr ComputePipelineSpecification& SetComputeShader(Shader& shader) { ComputeShader = &shader; return *this; }
        
        inline ComputePipelineSpecification& AddBindingLayout(BindingLayout& layout) { BindingLayouts.push_back(&layout); return *this; }
        inline constexpr ComputePipelineSpecification& SetPushConstants(const PushConstantSpecification& pushConstants) { PushConstants = pushConstants; return *this; }

        inline ComputePipelineSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }

    };

}