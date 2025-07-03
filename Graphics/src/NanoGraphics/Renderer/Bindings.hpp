#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/BindingsSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"
#include "NanoGraphics/Renderer/BufferSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanBindings.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Bindings.hpp"
#include "NanoGraphics/Platform/Dummy/DummyBindings.hpp"

#include <Nano/Nano.hpp>

#include <span>
#include <initializer_list>

namespace Nano::Graphics
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
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanBindingLayout>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12BindingLayout>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyBindingLayout>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyBindingLayout>
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
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanBindingSet>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12BindingSet>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyBindingSet>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyBindingSet>
        >;
    public:
        // Destructor
        ~BindingSet() = default;

        // Methods
        inline void SetItem(uint32_t slot, Image& image, const ImageSubresourceSpecification& subresources, uint32_t arrayIndex = 0) { m_Impl->SetItem(slot, image, subresources, arrayIndex); }
        inline void SetItem(uint32_t slot, Sampler& sampler, uint32_t arrayIndex = 0) { m_Impl->SetItem(slot, sampler, arrayIndex); }
        inline void SetItem(uint32_t slot, Buffer& buffer, const BufferRange& range, uint32_t arrayIndex = 0) { m_Impl->SetItem(slot, buffer, range, arrayIndex); }
        
        // TODO: Add batch uploading (back)

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
        using Type = Types::SelectorType<Information::RenderingAPI,
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanBindingSetPool>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::Dx12BindingSetPool>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyBindingSetPool>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyBindingSetPool>
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