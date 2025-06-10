#pragma once

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"
#include "NanoGraphics/Renderer/CommandListSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"

#include <unordered_map>

namespace Nano::Graphics
{
	class Image;
}

namespace Nano::Graphics::Internal
{

	class VulkanDevice;

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
	// VulkanCommandListPool
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanStateTracker
	{
	public:
		// Constructor & Destructor
		VulkanStateTracker(const VulkanDevice& device);
		~VulkanStateTracker();

		// Methods
		void Clear();

		void StartTracking(const Image& image, ImageSubresourceSpecification subresources, ResourceState currentState);

		void RequireImageState(Image& image, ImageSubresourceSpecification subresources, ResourceState state);
		
		void CommitBarriers(VkCommandBuffer cmdBuf);

		// Getters
		inline bool Contains(const Image& image) const { return m_ImageStates.contains(&image); }
		inline VulkanImageState& GetState(const Image& image) { return m_ImageStates[&image]; }

	private:
		const VulkanDevice& m_Device;

		std::unordered_map<const Image*, VulkanImageState> m_ImageStates = { };

		std::vector<ImageBarrier> m_ImageBarriers = { };
	};

}