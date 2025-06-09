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
	// VulkanExecutionRegion
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanExecutionRegion
	{
	public:
		// Constructor & Destructor
		VulkanExecutionRegion(const Swapchain& swapchain);
		~VulkanExecutionRegion();

		// Destruction methods
		void FreePool(CommandListPool& pool) const;

		// Methods

		// Internal methods

		// Internal getters
		inline const VulkanDevice& GetVulkanDevice() const { return m_Device; }

		inline VkSemaphore GetImageAvailableSemaphore(uint32_t frame) const { return m_ImageAvailableSemaphores[frame]; }

	private:
		const VulkanDevice& m_Device;

		std::array<VkSemaphore, Information::BackBufferCount> m_ImageAvailableSemaphores = { };
		
		VkSemaphore m_Timeline = VK_NULL_HANDLE;
		uint64_t m_Currentvalue = 0;

		std::array<uint64_t, Information::BackBufferCount> m_WaitValues = { };
	};

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
		
		inline const ExecutionRegion& GetExecutionRegion() const { return *reinterpret_cast<const ExecutionRegion*>(&m_ExecutionRegion); }

		// Internal getters
		inline const VulkanDevice& GetVulkanDevice() const { return m_Device; }

	private:
		// Private methods
		void ResolveFormatAndColourSpace(Format format, ColourSpace space);

	private:
		const VulkanDevice& m_Device;
		SwapchainSpecification m_Specification;

		VulkanExecutionRegion m_ExecutionRegion;

		VkSwapchainKHR m_SwapChain = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

		std::array<Nano::Memory::DeferredConstruct<VulkanImage, true>, Information::BackBufferCount> m_Images = { };

		uint32_t m_CurrentFrame = 0;
		uint32_t m_AcquiredImage = 0;
	};

}