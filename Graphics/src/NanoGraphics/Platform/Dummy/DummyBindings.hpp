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
        inline constexpr DummyBindingSet(BindingSetPool& pool, const BindingSetSpecification& specs) 
            : m_Specification(specs) { (void)pool; };
        inline constexpr ~DummyBindingSet() = default;

        // Methods
        inline constexpr void SetItem(uint32_t slot, Image& image, const ImageSubresourceSpecification& subresources, uint32_t arrayIndex) { (void)slot; (void)image; (void)subresources; (void)arrayIndex; }
        inline constexpr void SetItem(uint32_t slot, Sampler& sampler, uint32_t arrayIndex) { (void)slot; (void)sampler; (void)arrayIndex; }
        inline constexpr void SetItem(uint32_t slot, Buffer& buffer, const BufferRange& range, uint32_t arrayIndex) { (void)slot; (void)buffer; (void)range; (void)arrayIndex; }
    
        // Getters
        inline const BindingSetSpecification& GetSpecification() const { return m_Specification; }

    private:
        BindingSetSpecification m_Specification;
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