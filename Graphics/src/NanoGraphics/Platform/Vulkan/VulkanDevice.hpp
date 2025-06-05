#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/DeviceSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanContext.hpp"

#include <Nano/Nano.hpp>

#include <tuple>
#include <array>

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // VulkanDevice
    ////////////////////////////////////////////////////////////////////////////////////
    class VulkanDevice : public Traits::NoMove
    {
    public:
        // Constructors & Destructor
        VulkanDevice(const DeviceSpecification& specs);
        ~VulkanDevice();

        // Methods
        void Wait() const;

        // Internal Getters
        inline const VulkanContext& GetContext() const { return m_Context; }
        inline const VulkanAllocator& GetAllocator() const { return m_Allocator; }

    private:
        VulkanContext m_Context;
        VulkanAllocator m_Allocator;
    };

}