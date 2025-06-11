#pragma once

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include <cstdint>

namespace Nano::Graphics
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Flags
    ////////////////////////////////////////////////////////////////////////////////////
    enum class LoadOperation : uint8_t
    {
        None = 0,

        Clear,
        Load,
        DontCare = None,
    };

    enum class StoreOperation : uint8_t
    {
        None = 0,

        Store,
        DontCare = None,
    };

    enum class PipelineBindpoint : uint8_t
    {
        Graphics = 0,
        Compute,
        RayTracing
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // RenderpassSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct RenderpassSpecification
    {
    public:
        PipelineBindpoint Bindpoint = PipelineBindpoint::Graphics;

        ImageSpecification ColourSpecification = {};
        LoadOperation ColourLoadOperation = LoadOperation::Clear;
        StoreOperation ColourStoreOperation = StoreOperation::Store;
        Maths::Vec4<float> ColourClear = { 0.0f, 0.0f, 0.0f, 1.0f };
        ResourceState ColourImageStartState = ResourceState::Unknown;
        ResourceState ColourImageEndState = ResourceState::Present;

        ImageSpecification DepthSpecification = {};
        LoadOperation DepthLoadOperation = LoadOperation::Clear;
        StoreOperation DepthStoreOperation = StoreOperation::Store;
        float DepthClear = 1.0f;
        ResourceState DepthImageStartState = ResourceState::Unknown;
        ResourceState DepthImageEndState = ResourceState::DepthWrite;

    public:
        // Setters
        inline constexpr RenderpassSpecification& SetBindpoint(PipelineBindpoint point) { Bindpoint = point; return *this; }
        
        inline constexpr RenderpassSpecification& SetColourImageSpecification(const ImageSpecification& specs) { ColourSpecification = specs; return *this; }
        inline constexpr RenderpassSpecification& SetColourLoadOperation(LoadOperation operation) { ColourLoadOperation = operation; return *this; }
        inline constexpr RenderpassSpecification& SetColourStoreOperation(StoreOperation operation) { ColourStoreOperation = operation; return *this; }
        inline constexpr RenderpassSpecification& SetColourClear(const Maths::Vec4<float>& colour) { ColourClear = colour; return *this; }
        inline constexpr RenderpassSpecification& SetColourStartState(ResourceState state) { ColourImageStartState = state; return *this; }
        inline constexpr RenderpassSpecification& SetColourEndState(ResourceState state) { ColourImageEndState = state; return *this; }

        inline constexpr RenderpassSpecification& SetDepthImageSpecification(const ImageSpecification& specs) { DepthSpecification = specs; return *this; }
        inline constexpr RenderpassSpecification& SetDepthLoadOperation(LoadOperation operation) { DepthLoadOperation = operation; return *this; }
        inline constexpr RenderpassSpecification& SetDepthStoreOperation(StoreOperation operation) { DepthStoreOperation = operation; return *this; }
        inline constexpr RenderpassSpecification& SetDepthClear(float depth) { DepthClear = depth; return *this; }
        inline constexpr RenderpassSpecification& SetDepthStartState(ResourceState state) { DepthImageStartState = state; return *this; }
        inline constexpr RenderpassSpecification& SetDepthEndState(ResourceState state) { DepthImageEndState = state; return *this; }
    };

}