#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Renderer/BufferSpec.hpp"

#include <Nano/Nano.hpp>

#include <span>

namespace Nano::Graphics
{
    class Device;
}

namespace Nano::Graphics::Internal
{

    class DummyInputLayout;
    class DummyBuffer;

#if 1 //defined(NG_API_DUMMY)
    ////////////////////////////////////////////////////////////////////////////////////
    // DummyInputLayout
    ////////////////////////////////////////////////////////////////////////////////////
    class DummyInputLayout
    {
    public:
        // Constructor & Destructor
        inline constexpr DummyInputLayout(const Device& device, std::span<const VertexAttributeSpecification> attributes) { (void)device; (void)attributes; }
        constexpr ~DummyInputLayout() = default;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // DummyBuffer
    ////////////////////////////////////////////////////////////////////////////////////
    class DummyBuffer
    {
    public:
        // Constructor & Destructor
        inline constexpr DummyBuffer(const Device& device, const BufferSpecification& specs)
            : m_Specification(specs) { (void)device; }
        constexpr ~DummyBuffer() = default;

        // Getters
        inline constexpr const BufferSpecification& GetSpecification() const { return m_Specification; }

        inline constexpr size_t GetAlignment() const { return 2ull; }

    private:
        BufferSpecification m_Specification;
    };
#endif

}