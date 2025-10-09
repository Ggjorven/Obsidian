#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Renderer/API.hpp"
#include "Obsidian/Renderer/BindingsSpec.hpp"
#include "Obsidian/Renderer/ImageSpec.hpp"
#include "Obsidian/Renderer/BufferSpec.hpp"

#include "Obsidian/Platform/Vulkan/VulkanBindings.hpp"
#include "Obsidian/Platform/Dx12/Dx12Bindings.hpp"
#include "Obsidian/Platform/Dummy/DummyBindings.hpp"

#include <Nano/Nano.hpp>

#include <span>
#include <initializer_list>

namespace Obsidian
{

    class Device;
    class Image;
    class Sampler;
    class Buffer;
    class BindingSetPool;

    ////////////////////////////////////////////////////////////////////////////////////
    // BindingLayout
    ////////////////////////////////////////////////////////////////////////////////////
    class BindingLayout 
    {
    public:
        using Type = Nano::Types::SelectorType<Information::RenderingAPI,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanBindingLayout>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12BindingLayout>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyBindingLayout>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyBindingLayout>
        >;
    public:
        // Destructor
        ~BindingLayout() = default;

        // Getters
        inline bool IsBindless() const { return m_Impl->IsBindless(); }

    public: //private:
        // Constructor
        inline BindingLayout(const Device& device, const BindingLayoutSpecification& specs) { m_Impl.Construct(device, specs); }
        inline BindingLayout(const Device& device, const BindlessLayoutSpecification& specs) { m_Impl.Construct(device, specs); }
    
    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class Device;
        friend class APICaster;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // BindingSet
    ////////////////////////////////////////////////////////////////////////////////////
    class BindingSet
    {
    public:
        using Type = Nano::Types::SelectorType<Information::RenderingAPI,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanBindingSet>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12BindingSet>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyBindingSet>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyBindingSet>
        >;
    public:
        // Destructor
        ~BindingSet() = default;

        // Methods
        inline void SetItem(uint32_t slot, Image& image, const ImageSubresourceSpecification& subresources = ImageSubresourceSpecification(), uint32_t arrayIndex = 0) { m_Impl->SetItem(slot, image, subresources, arrayIndex); }
        inline void SetItem(uint32_t slot, Sampler& sampler, uint32_t arrayIndex = 0) { m_Impl->SetItem(slot, sampler, arrayIndex); }
        inline void SetItem(uint32_t slot, Buffer& buffer, const BufferRange& range = BufferRange(), uint32_t arrayIndex = 0) { m_Impl->SetItem(slot, buffer, range, arrayIndex); }

        // Getters
        inline const BindingSetSpecification& GetSpecification() const { return m_Impl->GetSpecification(); }

    public: //private:
        // Constructor
        inline BindingSet(BindingSetPool& pool, const BindingSetSpecification& specs) { m_Impl.Construct(pool, specs); }

    private:
        Internal::APIObject<Type> m_Impl = {};

        friend class BindingSetPool;
        friend class APICaster;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // BindingSetPool
    ////////////////////////////////////////////////////////////////////////////////////
    class BindingSetPool
    {
    public:
        using Type = Nano::Types::SelectorType<Information::RenderingAPI,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanBindingSetPool>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12BindingSetPool>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyBindingSetPool>,
            Nano::Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyBindingSetPool>
        >;
    public:
        // Destructor
        ~BindingSetPool() = default;

        // Creation methods // Note: Copy elision (RVO/NRVO) ensures object is constructed directly in the caller's stack frame.
        inline BindingSet CreateBindingSet(const BindingSetSpecification& specs) { return BindingSet(*this, specs); } // Note: BindingSets get destroyed by the pool

        // Getters
        inline const BindingSetPoolSpecification& GetSpecification() const { return m_Impl->GetSpecification(); }

    public: //private:
        // Constructor
        inline BindingSetPool(const Device& device, const BindingSetPoolSpecification& specs) { m_Impl.Construct(device, specs); }

    private:
        Internal::APIObject<Type> m_Impl;

        friend class Device;
        friend class APICaster;
    };

}