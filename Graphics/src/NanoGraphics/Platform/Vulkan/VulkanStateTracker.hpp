#pragma once

#include "NanoGraphics/Core/Core.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"
#include "NanoGraphics/Renderer/CommandListSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <unordered_map>

namespace Nano::Graphics
{
	class Image;
	class Buffer;
}

namespace Nano::Graphics::Internal
{

	class VulkanDevice;
	class VulkanStateTracker;

#if defined(NG_API_VULKAN)
	////////////////////////////////////////////////////////////////////////////////////
	// VulkanImageState
	////////////////////////////////////////////////////////////////////////////////////
	struct VulkanImageState
	{
	public:
		std::vector<ResourceState> SubresourceStates = { };
		ResourceState State = ResourceState::Unknown;

		bool EnableUavBarriers = true; // Note: Just to keep track of the fact that the specification specified it
		bool FirstUavBarrierPlaced = false;
		bool PermanentTransition = false;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanBufferState
	////////////////////////////////////////////////////////////////////////////////////
	struct VulkanBufferState
	{
	public:
		ResourceState State = ResourceState::Unknown;

		bool EnableUavBarriers = true; // Note: Just to keep track of the fact that the specification specified it
		bool FirstUavBarrierPlaced = false;
		bool PermanentTransition = false;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanCommandListPool
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanStateTracker : public Traits::NoCopy
	{
	public:
		// Constructor & Destructor
		VulkanStateTracker(const VulkanDevice& device);
		~VulkanStateTracker();

		// Methods
		void Clear() const;

		void StartTracking(const Image& image, ImageSubresourceSpecification subresources, ResourceState currentState) const;
		void StartTracking(const Buffer& buffer, ResourceState currentState) const;
		void StopTracking(const Image& image);
		void StopTracking(const Buffer& buffer);

		void RequireImageState(Image& image, ImageSubresourceSpecification subresources, ResourceState state) const;
		void RequireBufferState(Buffer& buffer, ResourceState state) const;
		
		void CommitBarriers(VkCommandBuffer cmdBuf) const;

		void ResolvePermanentState(Image& image, const ImageSubresourceSpecification& subresource) const;
		void ResolvePermanentState(Buffer& buffer) const;

		// Getters
		inline bool Contains(const Image& image) const { return m_ImageStates.contains(&image); }
		inline bool Contains(const Buffer& buffer) const { return m_BufferStates.contains(&buffer); }
		inline VulkanImageState& GetState(const Image& image) const { return m_ImageStates[&image]; }
		inline VulkanBufferState& GetState(const Buffer& buffer) const { return m_BufferStates[&buffer]; }

		ResourceState GetResourceState(const Image& image, ImageSubresourceSpecification subresource) const;
		ResourceState GetResourceState(const Buffer& buffer) const;

	private:
		const VulkanDevice& m_Device;

		mutable std::unordered_map<const Image*, VulkanImageState> m_ImageStates = { };
		mutable std::unordered_map<const Buffer*, VulkanBufferState> m_BufferStates = { };

		mutable std::vector<ImageBarrier> m_ImageBarriers = { };
		mutable std::vector<BufferBarrier> m_BufferBarriers = { };
	};
#endif

}