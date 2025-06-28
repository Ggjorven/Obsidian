#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12.hpp"

#include <vector>

namespace Nano::Graphics
{
    class Device;
}

namespace Nano::Graphics::Internal
{

    class Dx12Device;


#if defined(NG_API_DX12)
    ////////////////////////////////////////////////////////////////////////////////////
    // Helper
    ////////////////////////////////////////////////////////////////////////////////////
    using DescriptorHeapIndex = uint16_t;

    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12DescriptorHeap
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12DescriptorHeap // Note: There are no safety checks in this class, so overwriting is definitely possible (careful)
    {
    public:
        // Constructor & Destructor
        Dx12DescriptorHeap(const Device& device, uint16_t maxSize, D3D12_DESCRIPTOR_HEAP_TYPE type, bool isShaderVisible);
        ~Dx12DescriptorHeap();

        // Creation methods
        void CreateSRV(DescriptorHeapIndex index, const ImageSpecification& specs, const ImageSubresourceSpecification& subresources, ID3D12Resource* resource, Format format = Format::Unknown, ImageDimension dimension = ImageDimension::Unknown);
        void CreateSRV(DescriptorHeapIndex index, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, ID3D12Resource* resource);
        
        void CreateUAV(DescriptorHeapIndex index, const ImageSpecification& specs, const ImageSubresourceSpecification& subresources, ID3D12Resource* resource, Format format = Format::Unknown, ImageDimension dimension = ImageDimension::Unknown);
        void CreateUAV(DescriptorHeapIndex index, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, ID3D12Resource* resource);
        
        void CreateRTV(DescriptorHeapIndex index, const ImageSpecification& specs, const ImageSubresourceSpecification& subresources, ID3D12Resource* resource, Format format = Format::Unknown);
        void CreateRTV(DescriptorHeapIndex index, const D3D12_RENDER_TARGET_VIEW_DESC& desc, ID3D12Resource* resource);
        
        void CreateDSV(DescriptorHeapIndex index, const ImageSpecification& specs, const ImageSubresourceSpecification& subresources, ID3D12Resource* resource, bool isReadOnly = false);
        void CreateDSV(DescriptorHeapIndex index, const D3D12_DEPTH_STENCIL_VIEW_DESC& desc, ID3D12Resource* resource);
        
        void CreateSampler(DescriptorHeapIndex index, const SamplerSpecification& specs);
        void CreateSampler(DescriptorHeapIndex index, const D3D12_SAMPLER_DESC& desc);

        // Methods
        void Grow(uint32_t minNewSize);

        // Getters
        CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandleForIndex(DescriptorHeapIndex index) const;

    protected:
        const Dx12Device& m_Device;

        // Specification
        uint16_t m_MaxSize; // Amount of descriptors allocateable
        D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
        bool m_IsShaderVisible;

        DxPtr<ID3D12DescriptorHeap> m_DescriptorHeap = nullptr;
        
        // Helper
        CD3DX12_CPU_DESCRIPTOR_HANDLE m_CPUStart = {}; // Start of heap
        CD3DX12_GPU_DESCRIPTOR_HANDLE m_GPUStart = {}; // Start of heap
        uint32_t m_DescriptorSize = 0; // Size of m_Type descriptor (constant value)
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // Dx12DynamicDescriptorHeap
    ////////////////////////////////////////////////////////////////////////////////////
    class Dx12DynamicDescriptorHeap : private Dx12DescriptorHeap // Creating/freeing descriptor freely without caring about where in the heap (with reuse logic)
    {
    private:
        struct Entry
        {
        public:
            uint32_t Amount = 0;
            DescriptorHeapIndex Index = {};
        };
    public:
        // Constructor & Destructor
        Dx12DynamicDescriptorHeap(const Device& device, uint16_t maxSize, D3D12_DESCRIPTOR_HEAP_TYPE type, bool isShaderVisible);
        ~Dx12DynamicDescriptorHeap();

        // Creation methods // Note: Most are just passthrough functions
        DescriptorHeapIndex CreateSRV(const ImageSpecification& specs, const ImageSubresourceSpecification& subresources, ID3D12Resource* resource, Format format = Format::Unknown, ImageDimension dimension = ImageDimension::Unknown);
        DescriptorHeapIndex CreateSRV(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc, ID3D12Resource* resource);

        DescriptorHeapIndex CreateUAV(const ImageSpecification& specs, const ImageSubresourceSpecification& subresources, ID3D12Resource* resource, Format format = Format::Unknown, ImageDimension dimension = ImageDimension::Unknown);
        DescriptorHeapIndex CreateUAV(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc, ID3D12Resource* resource);

        DescriptorHeapIndex CreateRTV(const ImageSpecification& specs, const ImageSubresourceSpecification& subresources, ID3D12Resource* resource, Format format = Format::Unknown);
        DescriptorHeapIndex CreateRTV(const D3D12_RENDER_TARGET_VIEW_DESC& desc, ID3D12Resource* resource);

        DescriptorHeapIndex CreateDSV(const ImageSpecification& specs, const ImageSubresourceSpecification& subresources, ID3D12Resource* resource, bool isReadOnly = false);
        DescriptorHeapIndex CreateDSV(const D3D12_DEPTH_STENCIL_VIEW_DESC& desc, ID3D12Resource* resource);

        DescriptorHeapIndex CreateSampler(const SamplerSpecification& specs);
        DescriptorHeapIndex CreateSampler(const D3D12_SAMPLER_DESC& desc);

        void Free(DescriptorHeapIndex index);

        // Getters
        CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandleForIndex(DescriptorHeapIndex index) const;

    private:
        // Private methods
        DescriptorHeapIndex GetNextIndex();

    private:
        uint16_t m_Count = 0; // Amount of descriptors allocated
        DescriptorHeapIndex m_Offset = 0; // Current offset into heap

        std::vector<Entry> m_FreeEntries = {}; // Free'd up descriptors that can be reused.
    };
#endif

}