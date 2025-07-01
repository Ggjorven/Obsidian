#include "ngpch.h"
#include "Dx12Buffer.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12Device.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Resources.hpp"

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12InputLayout::Dx12InputLayout(const Device& device, std::span<const VertexAttributeSpecification> attributes)
        : m_Attributes(attributes.begin(), attributes.end())
    {
        CalculateOffsetsAndStride();

        m_InputElements.reserve(attributes.size() * 2);
        for (uint32_t i = 0; i < m_Attributes.size(); i++)
        {
            const VertexAttributeSpecification& attribute = m_Attributes[i];
            NG_ASSERT((attribute.ArraySize > 0), "[Dx12InputLayout] ArraySize must be larger than 0.");

            D3D12_INPUT_ELEMENT_DESC desc = {};
            desc.SemanticName = "TEXCOORD"; // Note: SPIRV-Cross always returns TEXCOORD
            desc.AlignedByteOffset = attribute.Offset;
            desc.Format = FormatToFormatMapping(attribute.VertexFormat).SRVFormat;
            desc.InputSlot = attribute.BufferIndex;
            desc.SemanticIndex = attribute.Location;

            if (attribute.IsInstanced)
            {
                desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
                desc.InstanceDataStepRate = 1;
            }
            else
            {
                desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                desc.InstanceDataStepRate = 0;
            }

            m_InputElements.push_back(desc);
        }
    }

    Dx12InputLayout::~Dx12InputLayout()
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Private methods
    ////////////////////////////////////////////////////////////////////////////////////
    void Dx12InputLayout::CalculateOffsetsAndStride()
    {
        uint32_t offset = 0;
        uint32_t maxOffsetEnd = 0;

        for (auto& attribute : m_Attributes)
        {
            if (attribute.Size == VertexAttributeSpecification::AutoSize)
                attribute.Size = FormatToFormatInfo(attribute.VertexFormat).BytesPerBlock;
            if (attribute.Offset == VertexAttributeSpecification::AutoOffset)
                attribute.Offset = offset;

            offset = attribute.Offset + attribute.Size;
            maxOffsetEnd = std::max(maxOffsetEnd, offset);
        }

        m_Stride = maxOffsetEnd;
    }

	////////////////////////////////////////////////////////////////////////////////////
	// Constructor & Destructor
	////////////////////////////////////////////////////////////////////////////////////
	Dx12Buffer::Dx12Buffer(const Device& device, const BufferSpecification& specs)
		: m_Specification(specs)
	{
        NG_ASSERT((specs.Size > 0), "[Dx12Buffer] Buffer size must be equal to 0.");

        const Dx12Device& dxDevice = *api_cast<const Dx12Device*>(&device);

        size_t size = m_Specification.Size;

        if (m_Specification.IsUnorderedAccessed) // Storage buffer
            size = Nano::Memory::AlignOffset(size, 256ull);

        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;
        if (m_Specification.IsUnorderedAccessed)
            flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

        D3D12_RESOURCE_STATES initialState = ResourceStateToD3D12ResourceStates(m_Specification.PermanentState);
        D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT;

        if (static_cast<bool>(m_Specification.CpuAccess & CpuAccessMode::Read))
            heapType = D3D12_HEAP_TYPE_READBACK;
        if (static_cast<bool>(m_Specification.CpuAccess & CpuAccessMode::Write)) // Note: We purposefully overwrite when we have write flag
            heapType = D3D12_HEAP_TYPE_UPLOAD;

        m_Allocation = dxDevice.GetAllocator().AllocateBuffer(m_Resource,
            size, initialState,
            flags, heapType
        );

        if constexpr (Information::Validation)
        {
            if (!m_Specification.DebugName.empty())
                dxDevice.GetContext().SetDebugName(m_Resource.Get(), m_Specification.DebugName);
        }
	}

	Dx12Buffer::~Dx12Buffer()
	{
	}

}