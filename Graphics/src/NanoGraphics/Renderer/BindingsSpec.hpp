#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"
#include "NanoGraphics/Renderer/BufferSpec.hpp"
#include "NanoGraphics/Renderer/ShaderSpec.hpp"
#include "NanoGraphics/Renderer/RenderpassSpec.hpp"

#include <Nano/Nano.hpp>

#include <cstdint>
#include <variant>
#include <string>

namespace Nano::Graphics
{

    class BindingLayout;

    class Image;
    class Sampler;
    class Buffer;

    ////////////////////////////////////////////////////////////////////////////////////
    // Flags
    ////////////////////////////////////////////////////////////////////////////////////
    enum class ResourceType : uint8_t
    {
        None = 0,

        Image,
        TextureSRV = Image,
        ImageUnordered,
        TextureUAV = ImageUnordered,
        StorageBuffer,
        TypedBufferSRV = StorageBuffer,
        StorageBufferUnordered,
        TypedBufferUAV = StorageBufferUnordered,
        UniformBuffer,
        ConstantBuffer = UniformBuffer,
        Sampler,
        //RayTracingAccelStruct,
        
        // Note: CombinedImageSampler is not supported, since only Vulkan supports it
        //       instead use a seperate texture2D ([]) and sampler ([]), and combine with sampler2D (GLSL)
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Structs
    ////////////////////////////////////////////////////////////////////////////////////
    struct BindingLayoutItem
    {
    public:
        inline constexpr static uint32_t MaxPushConstantSize = 128;
    public:
        ShaderStage Visibility = ShaderStage::None;

        uint32_t Slot = 0;
        ResourceType Type = ResourceType::None;
        
        uint16_t Size = 1; // Note: Either push constant size, descriptor array size/count.

        std::string DebugName = {};

    public:
        // Setters // Note: The Item setter are optional and can be set later via the BindingSet itself
        inline constexpr BindingLayoutItem& SetVisibility(ShaderStage visibility) { Visibility = visibility; return *this; }
        inline constexpr BindingLayoutItem& SetSlot(uint32_t slot) { Slot = slot; return *this; }
        inline constexpr BindingLayoutItem& SetType(ResourceType type) { Type = type; return *this; }
        inline constexpr BindingLayoutItem& SetSize(uint16_t size) { Size = size; return *this; }

        inline BindingLayoutItem& SetDebugName(const std::string& name) { DebugName = name; return *this; }

        // Operators
        inline constexpr bool operator == (const BindingLayoutItem& other) const { return ((Slot == other.Slot) && (Type == other.Type) && (Size == other.Size)); }
        inline constexpr bool operator != (const BindingLayoutItem& other) const { return !(*this == other); }

        // Getters
        inline constexpr uint32_t GetArraySize() const { return /*(Type == ResourceType::PushConstants) ? 1 :*/ Size; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // BindingLayoutSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct BindingLayoutSpecification
    {
    public:
        inline constexpr static size_t MaxBindings = 16;
    public:
        uint32_t RegisterSpace = 0; // In Vulkan maps to descriptor set index, in dx12 to space0/space1

        Nano::Memory::StaticVector<BindingLayoutItem, MaxBindings> Bindings;

        std::string DebugName = {};

    public:
        // Setters
        inline constexpr BindingLayoutSpecification& SetBindingSet(uint32_t index) { RegisterSpace = index; return *this; }
        inline constexpr BindingLayoutSpecification& SetRegisterSpace(uint32_t space) { RegisterSpace = space; return *this; }
        inline BindingLayoutSpecification& AddItem(const BindingLayoutItem& item) { Bindings.push_back(item); return *this; }
        inline BindingLayoutSpecification& SetDebugName(const std::string_view& name) { DebugName = name; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // BindlessLayoutSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct BindlessLayoutSpecification
    {
    public:
        inline constexpr static size_t MaxBindings = BindingLayoutSpecification::MaxBindings;
    public:
        uint32_t FirstSlot = 0;
        uint32_t MaxCapacity = 0;

        Nano::Memory::StaticVector<BindingLayoutItem, MaxBindings> Bindings;

        std::string DebugName = {};

    public:
        // Setters
        inline constexpr BindlessLayoutSpecification& SetFirstSlot(uint32_t slot) { FirstSlot = slot; return *this; }
        inline constexpr BindlessLayoutSpecification& SetMaxCapacity(uint32_t capacity) { MaxCapacity = capacity; return *this; }
        inline BindlessLayoutSpecification& AddItem(const BindingLayoutItem& item) { Bindings.push_back(item); return *this; }
        inline BindlessLayoutSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // BindingSetSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct BindingSetSpecification // Note: The register is set via the layout which is provided in the pool
    {
    public:
        std::string DebugName = {};

    public:
        // Setters
        inline BindingSetSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // BindingSetPoolSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct BindingSetPoolSpecification
    {
    public:
        BindingLayout* Layout = nullptr;

        uint32_t SetAmount = Information::FramesInFlight;

        std::string DebugName = {};
    
    public:
        // Setters
        inline constexpr BindingSetPoolSpecification& SetLayout(BindingLayout& layout) { Layout = &layout; return *this; }
        inline constexpr BindingSetPoolSpecification& SetSetAmount(uint32_t amount) { SetAmount = amount; return *this; }
        inline BindingSetPoolSpecification& SetDebugName(const std::string& name) { DebugName = name; return *this; }
    };

}