#pragma once

#include "NanoGraphics/Core/Information.hpp"

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
    class BindingLayout : public Traits::NoCopy
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
        inline bool IsBindless() const { return m_BindingLayout.IsBindless(); }

    public: //private:
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
            Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanBindingSet>,
            Types::EnumToType<Information::Structs::RenderingAPI::D3D12, Internal::DummyBindingSet>,
            Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyBindingSet>,
            Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyBindingSet>
        >;
    public:
        // Destructor
        ~BindingSet() = default;

        // Methods
        inline void Upload(Image& image, const ImageSubresourceSpecification& subresources, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex = 0) { m_BindingSet.Upload(image, subresources, resourceType, slot, arrayIndex); }
        inline void Upload(Sampler& sampler, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex = 0) { m_BindingSet.Upload(sampler, resourceType, slot, arrayIndex); }
        inline void Upload(Buffer& buffer, const BufferRange& range, ResourceType resourceType, uint32_t slot, uint32_t arrayIndex = 0) { m_BindingSet.Upload(buffer, range, resourceType, slot, arrayIndex); }
        
        inline void UploadList(std::span<const BindingSetUploadable> uploadables) { m_BindingSet.UploadList(uploadables); }
        inline void UploadList(std::initializer_list<BindingSetUploadable> uploadables) { m_BindingSet.UploadList(std::span<const BindingSetUploadable>(uploadables)); }

    public: //private:
        // Constructor
        inline BindingSet(BindingSetPool& pool)
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
        inline const BindingSetPoolSpecification& GetSpecification() const { return m_BindingSetPool.GetSpecification(); }

    public: //private:
        // Constructor
        inline BindingSetPool(const Device& device, const BindingSetPoolSpecification& specs)
            : m_BindingSetPool(device, specs) {}

    private:
        Type m_BindingSetPool;

        friend class Device;
    };

}