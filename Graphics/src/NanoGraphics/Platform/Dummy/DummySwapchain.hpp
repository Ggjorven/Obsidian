#pragma once

#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Core/Window.hpp" // Note: I don't really know if I want this here... // FIXME

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/API.hpp"
#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/SwapchainSpec.hpp"

#include "NanoGraphics/Platform/Dummy/DummyImage.hpp"

#include <type_traits>

namespace Nano::Graphics
{
	class Device;
	class Swapchain;
	class CommandListPool;
}

namespace Nano::Graphics::Internal
{

	class DummySwapchain;

#if 1 //defined(NG_API_DUMMY)
	////////////////////////////////////////////////////////////////////////////////////
	// DummySwapchain
	////////////////////////////////////////////////////////////////////////////////////
	class DummySwapchain
	{
	public:
		// Constructor & Destructor
		inline DummySwapchain(const Device& device, const SwapchainSpecification& specs)
			: m_Specification(specs)
		{
			ImageSpecification imageSpec = ImageSpecification()
				.SetImageDimension(ImageDimension::Image2D)
				.SetImageFormat(specs.RequestedFormat)
				.SetWidthAndHeight(m_Specification.WindowTarget->GetSize().x, m_Specification.WindowTarget->GetSize().y)
				.SetIsRenderTarget(true)
				.SetPermanentState(ResourceState::Present);

			for (auto& image : m_Images)
				image.Construct(device, imageSpec);
		}
		~DummySwapchain() = default;

		// Destruction methods
		inline constexpr void FreePool(CommandListPool& pool) const { (void)pool; }

		// Methods
		inline constexpr void Resize(uint32_t width, uint32_t height) { Resize(width, height, m_Specification.VSync, m_Specification.RequestedFormat, m_Specification.RequestedColourSpace); }
		inline constexpr void Resize(uint32_t width, uint32_t height, bool vsync, Format colourFormat, ColourSpace colourSpace)
		{
			// Update specification
			m_Specification.VSync = vsync;
			m_Specification.RequestedFormat = colourFormat;
			m_Specification.RequestedColourSpace = colourSpace;

			// Update images
			for (auto& image : m_Images)
			{
				DummyImage& dummyImage = *api_cast<DummyImage*>(&image);
				dummyImage.m_Specification.Width = width;
				dummyImage.m_Specification.Height = height;
			}
		}

		inline constexpr void AcquireNextImage() { m_CurrentFrame = (m_CurrentFrame + 1) % Information::FramesInFlight; }
		inline constexpr void Present() {}

		// Getters
		inline constexpr const SwapchainSpecification& GetSpecification() const { return m_Specification; }

		inline constexpr uint8_t GetCurrentFrame() const { return m_CurrentFrame; }
		inline constexpr uint8_t GetAcquiredImage() const { return m_CurrentFrame; }

		inline Image& GetImage(uint8_t frame) { return m_Images[frame].Get(); }
		inline const Image& GetImage(uint8_t frame) const { return m_Images[frame].Get(); }

		inline constexpr uint8_t GetImageCount() const { return static_cast<uint8_t>(m_Images.size()); }

	private:
		SwapchainSpecification m_Specification;
	
		std::array<Nano::Memory::DeferredConstruct<Image, true>, Information::FramesInFlight> m_Images = { };

		uint8_t m_CurrentFrame = 0;
	};
#endif

}