#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/BufferSpec.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Resources.hpp"

namespace Nano::Graphics
{
	class Device;
}

namespace Nano::Graphics::Internal
{

	class Dx12Device;
    class Dx12InputLayout;
    class Dx12Buffer;

#if defined(NG_API_DX12)
    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12InputLayout
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12InputLayout
    {
    public:
        // Constructor & Destructor
        Dx12InputLayout(const Device& device, std::span<const VertexAttributeSpecification> attributes);
        ~Dx12InputLayout();

        // Internal getters
        inline const std::vector<D3D12_INPUT_ELEMENT_DESC>& GetInputElements() const { return m_InputElements; }
    
    private:
        // Private methods
        void CalculateOffsetsAndStride();

        inline uint32_t GetStride() const { return m_Stride; }
        inline uint32_t GetNumAttributes() const { return static_cast<uint32_t>(m_Attributes.size()); }
        inline const VertexAttributeSpecification& GetAttributeSpecification(uint32_t index) const { return m_Attributes.at(index); }
    
    private:
        std::vector<VertexAttributeSpecification> m_Attributes;
        std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputElements = {};

        uint32_t m_Stride = 0;
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12Buffer
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12Buffer
    {
    public:
        // Constructor & Destructor
        Dx12Buffer(const Device& device, const BufferSpecification& specs);
        ~Dx12Buffer();

        // Getters
        inline const BufferSpecification& GetSpecification() const { return m_Specification; }

        // Internal getters
        inline DxPtr<ID3D12Resource> GetD3D12Resource() const { return m_Resource; }
        inline DxPtr<D3D12MA::Allocation> GetD3D12MAAllocation() const { return m_Allocation; }

    private:
        BufferSpecification m_Specification;

        DxPtr<ID3D12Resource> m_Resource = nullptr;
        DxPtr<D3D12MA::Allocation> m_Allocation = nullptr;

        friend class Dx12Device;
    };
#endif

}