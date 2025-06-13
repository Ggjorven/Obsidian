#pragma once

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include <cstdint>
#include <cmath>
#include <string_view>

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

    struct Viewport
    {
    public:
        float MinX = 0.0f, MaxX = 0.0f;
        float MinY = 0.0f, MaxY = 0.0f;
        float MinZ = 0.0f, MaxZ = 0.0f;

    public:
        // Constructors & Destructor
        constexpr Viewport() = default;
        inline constexpr Viewport(float width, float height) 
            : MinX(0.0f), MaxX(width), MinY(0.0f), MaxY(height), MinZ(0.0f), MaxZ(1.0f) {}
        inline constexpr Viewport(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
            : MinX(minX), MaxX(maxX), MinY(minY), MaxY(maxY), MinZ(minZ), MaxZ(maxZ) {}
        constexpr ~Viewport() = default;

        // Operators
        inline constexpr bool operator == (const Viewport& other) const { return ((MinX == other.MinX) && (MinY == other.MinY) && (MinZ == other.MinZ) && (MaxX == other.MaxX) && (MaxY == other.MaxY) && (MaxZ == other.MaxZ)); }
        inline constexpr bool operator != (const Viewport& other) const { return !(*this == other); }

        // Getters
        inline constexpr float GetWidth() const { return MaxX - MinX; }
        inline constexpr float GetHeight() const { return MaxY - MinY; }
    };

    struct ScissorRect
    {
    public:
        int MinX = 0, MaxX = 0;
        int MinY = 0, MaxY = 0;

    public:
        // Constructors & Destructor
        constexpr ScissorRect() = default;
        inline constexpr ScissorRect(int width, int height) 
            : MinX(0), MaxX(width), MinY(0), MaxY(height) {}
        inline constexpr ScissorRect(int minX, int maxX, int minY, int maxY) 
            : MinX(minX), MaxX(maxX), MinY(minY), MaxY(maxY) {}
        inline explicit ScissorRect(const Viewport& viewport)
            : MinX(static_cast<int>((std::floorf(viewport.MinX)))), MaxX(static_cast<int>((std::ceilf(viewport.MaxX)))), MinY(static_cast<int>((std::floorf(viewport.MinY)))), MaxY(static_cast<int>((std::ceilf(viewport.MaxY)))) {}
        constexpr ~ScissorRect() = default;

        // Operators
        inline constexpr bool operator == (const ScissorRect& other) const { return ((MinX == other.MinX) && (MinY == other.MinY) && (MaxX == other.MaxX) && (MaxY == other.MaxY)); }
        inline constexpr bool operator != (const ScissorRect& other) const { return !(*this == other); }

        // Getters
        inline constexpr int GetWidth() const { return MaxX - MinX; }
        inline constexpr int GetHeight() const { return MaxY - MinY; }
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
        ResourceState ColourImageStartState = ResourceState::Unknown;
        ResourceState ColourImageEndState = ResourceState::Present;

        ImageSpecification DepthSpecification = {};
        LoadOperation DepthLoadOperation = LoadOperation::Clear;
        StoreOperation DepthStoreOperation = StoreOperation::Store;
        ResourceState DepthImageStartState = ResourceState::Unknown;
        ResourceState DepthImageEndState = ResourceState::DepthWrite;

        std::string_view DebugName = {};

    public:
        // Setters
        inline constexpr RenderpassSpecification& SetBindpoint(PipelineBindpoint point) { Bindpoint = point; return *this; }
        
        inline constexpr RenderpassSpecification& SetColourImageSpecification(const ImageSpecification& specs) { ColourSpecification = specs; return *this; }
        inline constexpr RenderpassSpecification& SetColourLoadOperation(LoadOperation operation) { ColourLoadOperation = operation; return *this; }
        inline constexpr RenderpassSpecification& SetColourStoreOperation(StoreOperation operation) { ColourStoreOperation = operation; return *this; }
        inline constexpr RenderpassSpecification& SetColourStartState(ResourceState state) { ColourImageStartState = state; return *this; }
        inline constexpr RenderpassSpecification& SetColourEndState(ResourceState state) { ColourImageEndState = state; return *this; }

        inline constexpr RenderpassSpecification& SetDepthImageSpecification(const ImageSpecification& specs) { DepthSpecification = specs; return *this; }
        inline constexpr RenderpassSpecification& SetDepthLoadOperation(LoadOperation operation) { DepthLoadOperation = operation; return *this; }
        inline constexpr RenderpassSpecification& SetDepthStoreOperation(StoreOperation operation) { DepthStoreOperation = operation; return *this; }
        inline constexpr RenderpassSpecification& SetDepthStartState(ResourceState state) { DepthImageStartState = state; return *this; }
        inline constexpr RenderpassSpecification& SetDepthEndState(ResourceState state) { DepthImageEndState = state; return *this; }

        inline constexpr RenderpassSpecification& SetDebugName(std::string_view name) { DebugName = name; return *this; }
    };

}