#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/BindingsSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

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

    class DummyBindingLayout;
    class DummyBindingSet;
    class DummyBindingSetPool;

#if 1 //defined(NG_API_DUMMY)
    ////////////////////////////////////////////////////////////////////////////////////
    // DummyBindingLayout
    ////////////////////////////////////////////////////////////////////////////////////
    class DummyBindingLayout
    {
    public:
        // Constructors & Destructor
        inline constexpr DummyBindingLayout(const Device& device, const BindingLayoutSpecification& specs)
            : m_Specification(specs) { (void)device; }
        inline constexpr DummyBindingLayout(const Device& device, const BindlessLayoutSpecification& specs)
            : m_Specification(specs) { (void)device; }
        constexpr ~DummyBindingLayout() = default;

        // Getters
        inline bool IsBindless() const { return std::holds_alternative<BindlessLayoutSpecification>(m_Specification); }

    private:
        std::variant<BindingLayoutSpecification, BindlessLayoutSpecification> m_Specification;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // DummyBindingSet
    ////////////////////////////////////////////////////////////////////////////////////
    class DummyBindingSet
    {
    public:
        // Constructor & Destructor
        inline constexpr DummyBindingSet(BindingSetPool& pool) { (void)pool; };
        inline constexpr ~DummyBindingSet() = default;

        // Methods
        inline constexpr void Upload(Image& image, const ImageSubresourceSpecification& subresources, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex) { (void)image; (void)subresources; (void)resourceType; (void)slot; (void)arrayIndex; }
        inline constexpr void Upload(Sampler& sampler, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex) { (void)sampler; (void)resourceType; (void)slot; (void)arrayIndex; }
        inline constexpr void Upload(Buffer& buffer, const BufferRange& range, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex) { (void)buffer; (void)range; (void)resourceType; (void)slot; (void)arrayIndex; }

        void UploadList(std::span<const BindingSetUploadable> uploadables) { (void)uploadables; }
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // DummyBindingSetPool
    ////////////////////////////////////////////////////////////////////////////////////
    class DummyBindingSetPool
    {
    public:
        // Constructor & Destructor
        inline constexpr DummyBindingSetPool(const Device& device, const BindingSetPoolSpecification& specs)
            : m_Specification(specs) { (void)device; }
        inline constexpr ~DummyBindingSetPool() = default;

        // Getters
        inline constexpr const BindingSetPoolSpecification& GetSpecification() const { return m_Specification; }

    private:
        BindingSetPoolSpecification m_Specification;
    };
#endif

}