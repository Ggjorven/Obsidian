#pragma once

#include <functional>

namespace Nano::Graphics // TODO: OpenGL, D3D11, D3D12
{

	using Fn = std::function<void()>;

	////////////////////////////////////////////////////////////////////////////////////
	// Device
	////////////////////////////////////////////////////////////////////////////////////
	struct DeviceSpecification
	{
		Fn ErrorCallback;
		bool ValidationLayers;

		// Vulkan related
		const char** Extensions;

		DeviceSpecification& SetErrorCB(Fn);
		DeviceSpecification& SetValidationLayers(bool);
		DeviceSpecification& SetExtensions(const char**); // Vulkan related
	};

	class Device;
	Fn Device_Create(const DeviceSpecification& specs);

	// Vulkan: VulkanDevice -> Instance, Devices

	////////////////////////////////////////////////////////////////////////////////////
	// ...
	////////////////////////////////////////////////////////////////////////////////////

}