#include "obpch.h"
#include "Dx12Buffer.hpp"

#include "Obsidian/Core/Logging.hpp"
#include "Obsidian/Core/Information.hpp"
#include "Obsidian/Utils/Profiler.hpp"

#include "Obsidian/Platform/Dx12/Dx12Device.hpp"
#include "Obsidian/Platform/Dx12/Dx12Resources.hpp"

namespace Obsidian::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    Dx12InputLayout::Dx12InputLayout(const Device& device, std::span<const VertexAttributeSpecification> attributes)
        : m_Attributes(attributes.begin(), attributes.end())
    {
        (void)device;

        CalculateOffsetsAndStride();

        m_InputElements.reserve(attributes.size() * 2);
        for (uint32_t i = 0; i < m_Attributes.size(); i++)
        {
            const VertexAttributeSpecification& attribute = m_Attributes[i];
            OB_ASSERT((attribute.ArraySize > 0), "[Dx12InputLayout] ArraySize must be larger than 0.");

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
        OB_ASSERT((m_Specification.IsDynamic) || (m_Specification.Size != 0), "[Dx12Buffer] Buffer size must not be equal to 0.");

        const Dx12Device& dxDevice = *api_cast<const Dx12Device*>(&device);

        // Validation checks
        if constexpr (Information::Validation)
        {
            if (m_Specification.IsIndexBuffer || m_Specification.IsTexel)
                OB_ASSERT((m_Specification.BufferFormat != Format::Unknown), "[Dx12Buffer] A Texel or Index buffer must not have BufferFormat unknown.");
        }

        // Calculate alignment
        {
            // Regular buffers
            if (m_Specification.IsVertexBuffer)
            {
                m_Alignment = std::max(m_Alignment, BufferSpecification::DefaultVertexBufferAlignment);
            }
            if (m_Specification.IsIndexBuffer)
            {
                OB_ASSERT(((m_Specification.BufferFormat == Format::R16UInt) || (m_Specification.BufferFormat == Format::R32UInt)), "[Dx12Buffer] An index buffer must have format R16 or R32 float, a uint16 or uint32.");

                m_Alignment = std::max(m_Alignment, static_cast<size_t>(FormatToFormatInfo(m_Specification.BufferFormat).BytesPerBlock));
            }
            if (m_Specification.IsUniformBuffer)
            {
                m_Alignment = std::max(m_Alignment, BufferSpecification::DefaultUniformBufferAlignment);
            }
            if (m_Specification.IsTexel)
            {
                m_Alignment = std::max(m_Alignment, static_cast<size_t>(FormatToFormatInfo(m_Specification.BufferFormat).BytesPerBlock));
            }
            if (m_Specification.IsUnorderedAccessed)
            {
                m_Alignment = std::max(m_Alignment, BufferSpecification::DefaultStorageBufferAlignment);
            }

            // If no flags were set, we set it to the smallest alignment (index buffer)
            if (m_Alignment == 0)
            {
                m_Alignment = BufferSpecification::DefaultIndexBufferAlignment;
            }

            OB_ASSERT(((m_Alignment & (m_Alignment - 1)) == 0), "[Dx12Buffer] Internal error: Alignment must be a power of 2.");
        }

        // Size helper
        {
            if (m_Specification.IsDynamic)
            {
                OB_ASSERT((m_Specification.ElementCount != 0), "[Dx12Buffer] Element count must not equal zero when dynamic is enabled.");

                if constexpr (Information::Validation)
                {
                    if (m_Specification.Size != 0)
                        dxDevice.GetContext().Error("[Dx12Buffer] Dynamic buffer had custom size specified, on dynamic buffers the size must be specified via Stride and ElementCount.");
                }

                m_Specification.Size = (m_Specification.Stride + m_Alignment - 1) & ~(m_Alignment - 1);
                m_Specification.Size *= m_Specification.ElementCount;

                if constexpr (Information::Validation)
                {
                    if (!static_cast<bool>(m_Specification.CpuAccess & CpuAccessMode::Write))
                        dxDevice.GetContext().Warn("[VkBuffer] Creating a Dynamic buffer with out CpuAccessMode::Write flag. This is needed for in frame updates without a staging buffer and copying.");
                }
            }
            else
                m_Specification.Size = (m_Specification.Size + m_Alignment - 1) & ~(m_Alignment - 1);
        }

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
            m_Specification.Size, initialState,
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