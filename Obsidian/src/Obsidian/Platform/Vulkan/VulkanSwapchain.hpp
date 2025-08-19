#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Maths/Structs.hpp"

#include "Obsidian/Renderer/API.hpp"
#include "Obsidian/Renderer/ResourceSpec.hpp"
#include "Obsidian/Renderer/SwapchainSpec.hpp"

#include "Obsidian/Platform/Vulkan/Vulkan.hpp"
#include "Obsidian/Platform/Vulkan/VulkanResources.hpp"
#include "Obsidian/Platform/Vulkan/VulkanImage.hpp"

#include <Nano/Nano.hpp>

#include <type_traits>

namespace Obsidian
{
	class Device;
	class Swapchain;
	class CommandListPool;
}

namespace Obsidian::Internal
{

	class VulkanDevice;
	class VulkanCommandList;
	class VulkanSwapchain;

	struct SwapchainSupportDetails;

#if defined(OB_API_VULKAN)
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

		inline uint8_t GetCurrentFrame() const { return m_CurrentFrame; }
		inline uint8_t GetAcquiredImage() const { return static_cast<uint8_t>(m_AcquiredImage); }

		inline Image& GetImage(uint8_t index) { return m_Images[index].Get(); }
		inline const Image& GetImage(uint8_t index) const { return m_Images[index].Get(); }

		inline uint8_t GetImageCount() const { return static_cast<uint8_t>(m_Images.size()); }
		
		// Internal getters
		inline VkSwapchainKHR GetVkSwapchain() const { return m_Swapchain; }
		inline VkSurfaceKHR GetVkSurface() const { return m_Surface; }
		inline VkSemaphore GetVkTimelineSemaphore() const { return m_TimelineSemaphore; }

		inline VkSemaphore GetVkImageAvailableSemaphore(uint8_t frame) const { return m_ImageAvailableSemaphores[frame]; }
		inline VkSemaphore GetVkSwapchainPresentableSemaphore(uint8_t index) const { return m_SwapchainPresentableSemaphores[index]; }

		inline const VulkanDevice& GetVulkanDevice() const { return m_Device; }

	private:
		// Private methods
		void ResolveFormatAndColourSpace(const SwapchainSupportDetails& details, Format format, ColourSpace space);

	private:
		const VulkanDevice& m_Device;
		SwapchainSpecification m_Specification;

		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

		Nano::Memory::StaticVector<Nano::Memory::DeferredConstruct<Image, true>, Information::MaxImageCount> m_Images = { };
		std::array<VkSemaphore, Information::FramesInFlight> m_ImageAvailableSemaphores = { };
		Nano::Memory::StaticVector<VkSemaphore, Information::MaxImageCount> m_SwapchainPresentableSemaphores = { };

		VkSemaphore m_TimelineSemaphore = VK_NULL_HANDLE;
		uint64_t m_CurrentTimelineValue = 0;

		std::array<uint64_t, Information::FramesInFlight> m_WaitTimelineValues = { };
		std::unordered_map<const VulkanCommandList*, uint64_t> m_CommandListSemaphoreValues = { };

		uint8_t m_CurrentFrame = 0;
		uint32_t m_AcquiredImage = 0;

		// Resizing utilities
		VkCommandPool m_ResizePool = VK_NULL_HANDLE;
		VkCommandBuffer m_ResizeCommand = VK_NULL_HANDLE;

		friend class VulkanDevice;
	};
#endif

}