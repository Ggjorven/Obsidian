#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/SwapchainSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanResources.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanImage.hpp"

#include <type_traits>

namespace Nano::Graphics
{
	class Device;
}

namespace Nano::Graphics::Internal
{

	class VulkanDevice;

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanSwapchain
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanSwapchain : public Traits::NoMove, public Traits::NoCopy
	{
	public:
		// Constructor & Destructor
		VulkanSwapchain(const Device& device, const SwapchainSpecification& specs);
		~VulkanSwapchain();

		// Methods
		void Resize(uint32_t width, uint32_t height);
		void Resize(uint32_t width, uint32_t height, bool vsync, Format colourFormat, ColourSpace colourSpace);

		void AcquireNextImage();
		void Present();

		// Getters
		inline const SwapchainSpecification& GetSpecification() const { return m_Specification; }
		
		// Internal getters

	private:
		// Private methods
		void ResolveFormatAndColourSpace(Format format, ColourSpace space);

	private:
		const VulkanDevice& m_Device;
		SwapchainSpecification m_Specification;

		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

		std::array<Nano::Memory::DeferredConstruct<VulkanImage, true>, Information::BackBufferCount> m_Images = { };

		std::array<VkSemaphore, Information::BackBufferCount> m_ImageAvailableSemaphores = { };

		uint32_t m_CurrentFrame = 0;
		uint32_t m_AcquiredImage = 0;
	};

}