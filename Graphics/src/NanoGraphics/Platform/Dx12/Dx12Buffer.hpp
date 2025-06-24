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
    //class Dx12InputLayout
    //{
    //public:
    //    // Constructor & Destructor
    //    Dx12InputLayout(const Device& device, std::span<const VertexAttributeSpecification> attributes);
    //    ~Dx12InputLayout();
    //
    //private:
    //    // Private methods
    //    void CalculateOffsetsAndStride();
    //
    //private:
    //};

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

        // Note: Maybe in the future add BufferViews like ImageViews

        friend class Dx12Device;
    };
#endif

}