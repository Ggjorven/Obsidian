#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/BindingsSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"
#include "NanoGraphics/Renderer/BufferSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/VulkanBindings.hpp"
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
            Types::EnumToType<Information::Structs::RenderingAPI::D3D12, Internal::DummyBindingLayout>,
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
            Types::EnumToType<Information::Structs::RenderingAPI::D3D12, Internal::DummyBindingSet>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyBindingSet>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyBindingSet>
        >;
    public:
        // Destructor
        ~BindingSet() = default;

        // Methods
        inline void Upload(Image& image, const ImageSubresourceSpecification& subresources, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex = 0) { m_Impl->Upload(image, subresources, resourceType, slot, arrayIndex); }
        inline void Upload(Sampler& sampler, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex = 0) { m_Impl->Upload(sampler, resourceType, slot, arrayIndex); }
        inline void Upload(Buffer& buffer, const BufferRange& range, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex = 0) { m_Impl->Upload(buffer, range, resourceType, slot, arrayIndex); }
        
        inline void UploadList(std::span<const BindingSetUploadable> uploadables) { m_Impl->UploadList(uploadables); }
        inline void UploadList(std::initializer_list<BindingSetUploadable> uploadables) { m_Impl->UploadList(std::span<const BindingSetUploadable>(uploadables)); }

    public: //private:
        // Constructor
        inline BindingSet(BindingSetPool& pool) { m_Impl.Construct(pool); }

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
            Types::EnumToType<Information::Structs::RenderingAPI::D3D12, Internal::DummyBindingSetPool>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyBindingSetPool>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyBindingSetPool>
        >;
    public:
        // Destructor
        ~BindingSetPool() = default;

        // Creation methods // Note: Copy elision (RVO/NRVO) ensures object is constructed directly in the caller's stack frame.
        inline BindingSet CreateBindingSet() { return BindingSet(*this); } // Note: BindingSets get destroyed by the pool

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