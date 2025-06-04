#pragma once

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include "NanoGraphics/Platform/Vulkan/Vulkan.hpp"
#include "NanoGraphics/Platform/Vulkan/VulkanDevice.hpp"

#include <cstdint>
#include <vector>
#include <optional>

namespace Nano::Graphics
{
	class Device;
}

namespace Nano::Graphics::Internal
{

	////////////////////////////////////////////////////////////////////////////////////
	// Image
	////////////////////////////////////////////////////////////////////////////////////
	class VulkanImage
	{
	public:
		// Constructor & Destructor
		VulkanImage(const Device& device, const ImageSpecification& specs);
		~VulkanImage();

		// Getters
		inline const ImageSpecification& GetSpecification() const { return m_Specification; }

	private:

	private:
		const VulkanDevice& m_Device;
		ImageSpecification m_Specification;
	};

}