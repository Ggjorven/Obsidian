#pragma once

#include "NanoGraphics/Core/Information.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/API.hpp"
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
}

namespace Nano::Graphics::Internal
{

	class VulkanDevice;
	class VulkanCommandList;
	class VulkanSwapchain;

	struct SwapchainSupportDetails;

#if defined(NG_API_VULKAN)
	////////////////////////////////////////////////////////////////////////////////////
	// VulkanSwapchain
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanSwapchain
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
		uint64_t GetPreviousCommandListWaitValue(const VulkanCommandList& commandList) const;
		uint64_t RetrieveCommandListWaitValue(const VulkanCommandList& commandList);

		// Getters
		inline const SwapchainSpecification& GetSpecification() const { return m_Specification; }

		inline uint32_t GetCurrentFrame() const { return m_CurrentFrame; }
		inline Image& GetImage(uint8_t frame) { return m_Images[frame].Get(); }
		inline const Image& GetImage(uint8_t frame) const { return m_Images[frame].Get(); }
		
		// Internal getters
		inline VkSwapchainKHR GetVkSwapchain() const { return m_Swapchain; }
		inline VkSurfaceKHR GetVkSurface() const { return m_Surface; }
		inline VkSemaphore GetVkTimelineSemaphore() const { return m_TimelineSemaphore; }

		inline VkSemaphore GetVkImageAvailableSemaphore(uint32_t frame) const { return m_ImageAvailableSemaphores[frame]; }
		inline VkSemaphore GetVkSwapchainPresentableSemaphore(uint32_t frame) const { return m_SwapchainPresentableSemaphores[frame]; }

		inline uint32_t GetAcquiredImage() const { return m_AcquiredImage; }

		inline const VulkanDevice& GetVulkanDevice() const { return m_Device; }

	private:
		// Private methods
		void ResolveFormatAndColourSpace(const SwapchainSupportDetails& details, Format format, ColourSpace space);

	private:
		const VulkanDevice& m_Device;
		SwapchainSpecification m_Specification;

		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

		Nano::Memory::StaticVector<Nano::Memory::DeferredConstruct<Image, true>, Information::BackBufferUpperLimit> m_Images = { };
		std::array<VkSemaphore, Information::BackBufferCount> m_ImageAvailableSemaphores = { };
		std::array<VkSemaphore, Information::BackBufferCount> m_SwapchainPresentableSemaphores = { };

		VkSemaphore m_TimelineSemaphore = VK_NULL_HANDLE;
		uint64_t m_CurrentTimelineValue = 0;

		std::array<uint64_t, Information::BackBufferCount> m_WaitTimelineValues = { };
		std::unordered_map<const VulkanCommandList*, uint64_t> m_CommandListSemaphoreValues = { };

		uint32_t m_CurrentFrame = 0;
		uint32_t m_AcquiredImage = 0;

		// Resizing utilities
		VkCommandPool m_ResizePool = VK_NULL_HANDLE;
		VkCommandBuffer m_ResizeCommand = VK_NULL_HANDLE;

		friend class VulkanDevice;
	};
#endif

}