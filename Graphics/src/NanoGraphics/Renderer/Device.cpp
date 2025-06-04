#include "ngpch.h"
#include "Device.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

namespace Nano::Graphics
{

	////////////////////////////////////////////////////////////////////////////////////
	// Creation methods
	////////////////////////////////////////////////////////////////////////////////////
	Image Device::CreateImage(const ImageSpecification& specs) const
	{
		return Image(*this, specs);
	}

}