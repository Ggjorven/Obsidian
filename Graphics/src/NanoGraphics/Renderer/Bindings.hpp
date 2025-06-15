#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/BindingsSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanBindings.hpp"

#include <Nano/Nano.hpp>

namespace Nano::Graphics
{

    class Device;
    class BindingSetPool;

    ////////////////////////////////////////////////////////////////////////////////////
    // BindingLayout
    ////////////////////////////////////////////////////////////////////////////////////
    class BindingLayout : public Traits::NoCopy
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanBindingLayout>
        >;
    public:
        // Destructor
        ~BindingLayout() = default;

        // Getters
        inline bool IsBindless() const { return m_BindingLayout.IsBindless(); }

    private:
        // Constructor
        BindingLayout(const Device& device, const BindingLayoutSpecification& specs)
            : m_BindingLayout(device, specs) {}
        BindingLayout(const Device& device, const BindlessLayoutSpecification& specs)
            : m_BindingLayout(device, specs) {}

    private:
        Type m_BindingLayout;

        friend class Device;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // BindingSet
    ////////////////////////////////////////////////////////////////////////////////////
    class BindingSet : public Traits::NoCopy
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanBindingSet>
        >;
    public:
        // Destructor
        ~BindingSet() = default;

    private:
        // Constructor
        BindingSet(BindingSetPool& pool)
            : m_BindingSet(pool) {}

    private:
        Type m_BindingSet;

        friend class BindingSetPool;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // BindingSetPool
    ////////////////////////////////////////////////////////////////////////////////////
    class BindingSetPool : public Traits::NoCopy
    {
    public:
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanBindingSetPool>
        >;
    public:
        // Destructor
        ~BindingSetPool() = default;

        // Creation methods // Note: Copy elision (RVO/NRVO) ensures object is constructed directly in the caller's stack frame.
        inline BindingSet CreateBindingSet() { return BindingSet(*this); } // Note: BindingSets get destroyed by the pool

    private:
        // Constructor
        BindingSetPool(const Device& device, const BindingSetPoolSpecification& specs)
            : m_BindingSetPool(device, specs) {}

    private:
        Type m_BindingSetPool;

        friend class Device;
    };

}