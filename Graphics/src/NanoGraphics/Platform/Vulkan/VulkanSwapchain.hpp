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
	class Swapchain;
	class CommandListPool;
	class CommandList;
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

		// Destruction methods
		void FreePool(CommandListPool& pool) const;

		// Methods
		void Resize(uint32_t width, uint32_t height);
		void Resize(uint32_t width, uint32_t height, bool vsync, Format colourFormat, ColourSpace colourSpace);

		void AcquireNextImage();
		void Present();

		// Internal methods
		uint64_t GetPreviousCommandListWaitValue(CommandList& commandList) const;
		uint64_t RetrieveCommandListWaitValue(CommandList& commandList);
		inline uint64_t GetImageAvaibleSemaphoreValue() const { return m_ImageAvailableValue; }

		// Getters
		inline const SwapchainSpecification& GetSpecification() const { return m_Specification; }
		
		// Internal getters
		inline VkSwapchainKHR GetVkSwapchain() const { return m_Swapchain; }
		inline VkSurfaceKHR GetVkSurface() const { return m_Surface; }

		inline const VulkanDevice& GetVulkanDevice() const { return m_Device; }

	private:
		// Private methods
		void ResolveFormatAndColourSpace(Format format, ColourSpace space);

	private:
		const VulkanDevice& m_Device;
		SwapchainSpecification m_Specification;

		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

		std::array<Nano::Memory::DeferredConstruct<VulkanImage, true>, Information::BackBufferCount> m_Images = { };
		std::array<VkSemaphore, Information::BackBufferCount> m_ImageAvailableSemaphores = { };

		VkSemaphore m_TimelineSemaphore = VK_NULL_HANDLE;
		uint64_t m_CurrentTimelineValue = 0;
		uint64_t m_ImageAvailableValue = 0;
		std::array<uint64_t, Information::BackBufferCount> m_WaitTimelineValues = { };
		std::unordered_map<CommandList*, uint64_t> m_CommandListSemaphoreValues = { };

		uint32_t m_CurrentFrame = 0;
		uint32_t m_AcquiredImage = 0;

		friend class VulkanDevice;
	};

}