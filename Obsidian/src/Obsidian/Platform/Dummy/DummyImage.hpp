#pragma once

#include "Obsidian/Maths/Structs.hpp"

#include "Obsidian/Renderer/ResourceSpec.hpp"
#include "Obsidian/Renderer/ImageSpec.hpp"

#include <type_traits>

namespace Obsidian
{
	class Device;
	class Image;
}

namespace Obsidian::Internal
{

	class DummySwapchain;
	class DummyImage;
	class DummyStagingImage;
	class DummySampler;

#if 1 //defined(OB_API_DUMMY)
	////////////////////////////////////////////////////////////////////////////////////
	// DummyImage
	////////////////////////////////////////////////////////////////////////////////////
	class DummyImage 
	{
	public:
		// Constructors & Destructor
		inline constexpr DummyImage(const Device& device, const ImageSpecification& specs)
			: m_Specification(specs) { (void)device; }
		constexpr ~DummyImage() = default;

		// Methods
		inline constexpr void Resize(uint32_t width, uint32_t height) { (void)width; (void)height; }

		// Getters
		inline constexpr const ImageSpecification& GetSpecification() const { return m_Specification; }

	private:
		ImageSpecification m_Specification;

		friend class DummySwapchain;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// DummyStagingImage
	////////////////////////////////////////////////////////////////////////////////////
	class DummyStagingImage
	{
	public:
		// Constructor & Destructor
		inline constexpr DummyStagingImage(const Device& device, const ImageSpecification& specs, CpuAccessMode cpuAccessMode)
			: m_Specification(specs) { (void)device; (void)cpuAccessMode; }
		constexpr ~DummyStagingImage() = default;

		// Getters
		inline constexpr const ImageSpecification& GetSpecification() const { return m_Specification; }

	private:
		ImageSpecification m_Specification;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// DummySampler
	////////////////////////////////////////////////////////////////////////////////////
	class DummySampler
	{
	public:
		// Constructor & Destructor
		inline constexpr DummySampler(const Device& device, const SamplerSpecification& specs)
			: m_Specification(specs) { (void)device; }
		constexpr ~DummySampler() = default;

		// Getters
		inline constexpr const SamplerSpecification& GetSpecification() const { return m_Specification; }

	private:
		SamplerSpecification m_Specification;
	};
#endif

}