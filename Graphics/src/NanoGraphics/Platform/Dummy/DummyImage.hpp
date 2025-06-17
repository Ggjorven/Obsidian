#pragma once

#include "NanoGraphics/Maths/Structs.hpp"

#include "NanoGraphics/Renderer/ResourceSpec.hpp"
#include "NanoGraphics/Renderer/ImageSpec.hpp"

#include <type_traits>

namespace Nano::Graphics
{
	class Device;
	class Image;
}

namespace Nano::Graphics::Internal
{

	class DummySwapchain;
	class DummyImage;
	class DummyStagingImage;
	class DummySampler;

#if 1 //defined(NG_API_DUMMY)
	////////////////////////////////////////////////////////////////////////////////////
	// DummyImage
	////////////////////////////////////////////////////////////////////////////////////
	class DummyImage : public Traits::NoCopy
	{
	public:
		// Constructors & Destructor
		inline constexpr DummyImage(const Device& device, const ImageSpecification& specs)
			: m_Specification(specs) { (void)device; }
		constexpr ~DummyImage() = default;

		// Getters
		inline constexpr const ImageSpecification& GetSpecification() const { return m_Specification; }

	private:
		ImageSpecification m_Specification;

		friend class DummySwapchain;
	};

	////////////////////////////////////////////////////////////////////////////////////
	// DummyStagingImage
	////////////////////////////////////////////////////////////////////////////////////
	class DummyStagingImage : public Traits::NoCopy
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
	class DummySampler : public Traits::NoCopy
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