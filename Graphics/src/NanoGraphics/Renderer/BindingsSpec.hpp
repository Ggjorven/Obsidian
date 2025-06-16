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
#include <string_view>

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
        ImageUnordered,
        StorageBuffer,
        StorageBufferUnordered,
        UniformBuffer,
        // TODO: Add volatile/dynamic uniform
        Sampler,
        PushConstants,
        //RayTracingAccelStruct,
        
        // Note: CombinedImageSampler is not supported, since only Vulkan supports it
        //       instead use a seperate texture2D ([]) and sampler ([]), and combine with sampler2D
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

        std::string_view DebugName = {};

    public:
        // Setters
        inline constexpr BindingLayoutItem& SetVisibility(ShaderStage visibility) { Visibility = visibility; return *this; }
        inline constexpr BindingLayoutItem& SetSlot(uint32_t slot) { Slot = slot; return *this; }
        inline constexpr BindingLayoutItem& SetType(ResourceType type) { Type = type; return *this; }
        inline constexpr BindingLayoutItem& SetSize(uint16_t size) { Size = size; return *this; }

        inline constexpr BindingLayoutItem& SetDebugName(std::string_view name) { DebugName = name; return *this; }

        // Operators
        inline constexpr bool operator == (const BindingLayoutItem& other) const { return ((Slot == other.Slot) && (Type == other.Type) && (Size == other.Size)); }
        inline constexpr bool operator != (const BindingLayoutItem& other) const { return !(*this == other); }

        // Methods
        inline constexpr uint32_t GetArraySize() const { return (Type == ResourceType::PushConstants) ? 1 : Size; }
    };

    struct VulkanBindingOffsets // Note: Describes compile-time settings for HLSL -> SPIR-V register allocation.
    {
    public:
        uint32_t ShaderResource = 0;
        uint32_t Sampler = 128;
        uint32_t UniformBuffer = 256;
        uint32_t UnorderedAccess = 384;

    public:
        // Setters
        inline constexpr VulkanBindingOffsets& SetShaderResourceOffset(uint32_t offset) { ShaderResource = offset; return *this; }
        inline constexpr VulkanBindingOffsets& SetSamplerOffset(uint32_t offset) { Sampler = offset; return *this; }
        inline constexpr VulkanBindingOffsets& SetConstantBufferOffset(uint32_t offset) { UniformBuffer = offset; return *this; }
        inline constexpr VulkanBindingOffsets& SetUnorderedAccessViewOffset(uint32_t offset) { UnorderedAccess = offset; return *this; }
    };

    struct BindingSetUploadable
    {
    public:
        std::variant<Image*, Sampler*, Buffer*> Element = {};
        std::variant<ImageSubresourceSpecification, BufferRange> Range = {};

        ResourceType Type = ResourceType::None;
        
        uint32_t Slot = 0;
        uint32_t ArrayIndex = 0;

    public:
        // Setters
        inline BindingSetUploadable& SetElement(Image& image, const ImageSubresourceSpecification& subresources = ImageSubresourceSpecification(0, ImageSubresourceSpecification::AllMipLevels, 0, ImageSubresourceSpecification::AllArraySlices)) { Element = &image; Range = subresources; return *this; }
        inline BindingSetUploadable& SetElement(Sampler& sampler) { Element = &sampler; return *this; }
        inline BindingSetUploadable& SetElement(Buffer& buffer, const BufferRange& range = BufferRange(BufferRange::FullSize, 0)) { Element = &buffer; Range = range; return *this; }

        inline constexpr BindingSetUploadable& SetResourceType(ResourceType type) { Type = type; return *this; }
        inline constexpr BindingSetUploadable& SetSlot(uint32_t slot) { Slot = slot; return *this; }
        inline constexpr BindingSetUploadable& SetArrayIndex(uint32_t index) { ArrayIndex = index; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // BindingLayoutSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct BindingLayoutSpecification
    {
    public:
        //ShaderStage Visibility = ShaderStage::None;

        uint32_t RegisterSpace = 0; // In vulkan maps to descriptor set index. Other API's not implemented.
        bool RegisterSpaceIsDescriptorSet = false;

        std::vector<BindingLayoutItem> Bindings;
        VulkanBindingOffsets BindingOffsets = {};

        std::string_view DebugName = {};

    public:
        // Setters
        //inline constexpr BindingLayoutSpecification& SetVisibility(ShaderStage stage) { Visibility = stage; return *this; }
        inline constexpr BindingLayoutSpecification& SetRegisterSpace(uint32_t space) { RegisterSpace = space; return *this; }
        inline constexpr BindingLayoutSpecification& SetRegisterSpaceIsDescriptorSet(bool enabled) { RegisterSpaceIsDescriptorSet = enabled; return *this; }
        inline constexpr BindingLayoutSpecification& AddItem(const BindingLayoutItem& item) { Bindings.push_back(item); return *this; }
        inline constexpr BindingLayoutSpecification& SetBindingOffsets(const VulkanBindingOffsets& offsets) { BindingOffsets = offsets; return *this; }
        inline constexpr BindingLayoutSpecification& SetDebugName(std::string_view name) { DebugName = name; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // BindlessLayoutSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct BindlessLayoutSpecification
    {
    public:
        inline constexpr static size_t MaxBindLessRegisterSpaces = 16;
    public:
        //ShaderStage Visibility = ShaderStage::None;

        uint32_t FirstSlot = 0;
        uint32_t MaxCapacity = 0;

        Nano::Memory::StaticVector<BindingLayoutItem, MaxBindLessRegisterSpaces> RegisterSpaces;

        std::string_view DebugName = {};

    public:
        // Setters
        //inline constexpr BindlessLayoutSpecification& SetVisibility(ShaderStage stage) { Visibility = stage; return *this; }
        inline constexpr BindlessLayoutSpecification& SetFirstSlot(uint32_t slot) { FirstSlot = slot; return *this; }
        inline constexpr BindlessLayoutSpecification& SetMaxCapacity(uint32_t capacity) { MaxCapacity = capacity; return *this; }
        inline BindlessLayoutSpecification& AddRegisterSpace(const BindingLayoutItem& item) { RegisterSpaces.push_back(item); return *this; }
        inline constexpr BindlessLayoutSpecification& SetDebugName(std::string_view name) { DebugName = name; return *this; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // BindingSetPoolSpecification
    ////////////////////////////////////////////////////////////////////////////////////
    struct BindingSetPoolSpecification
    {
    public:
        BindingLayout* Layout = nullptr;

        uint32_t SetAmount = Information::BackBufferCount;

        std::string_view DebugName = {};
    
    public:
        // Setters
        inline constexpr BindingSetPoolSpecification& SetLayout(BindingLayout& layout) { Layout = &layout; return *this; }
        inline constexpr BindingSetPoolSpecification& SetSetAmount(uint32_t amount) { SetAmount = amount; return *this; }
        inline constexpr BindingSetPoolSpecification& SetDebugName(std::string_view name) { DebugName = name; return *this; }
    };

}