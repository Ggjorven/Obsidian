#pragma once

#include "NanoGraphics/Platform/Vulkan/VulkanImGuiRenderer.hpp"
#include "NanoGraphics/Platform/Dummy/DummyImGuiRenderer.hpp"

#include <Nano/Nano.hpp>

#include <imgui.h>

namespace Nano::Graphics
{

	class Device;
	class Swapchain;
	class Renderpass;
	class CommandList;

	////////////////////////////////////////////////////////////////////////////////////
	// ImGuiRenderer
	////////////////////////////////////////////////////////////////////////////////////
	class ImGuiRenderer // Static imgui wrapper for ease of use with multiple APIs // Note: Renderpass must be manually resized and Started/Ended using a CommandList
	{
	public:
		using Type = Types::SelectorType<Information::RenderingAPI,
			Types::EnumToType<Information::Structs::RenderingAPI::Vulkan, Internal::VulkanImGuiRenderer>,
			Types::EnumToType<Information::Structs::RenderingAPI::Dx12, Internal::DummyImGuiRenderer>,
			Types::EnumToType<Information::Structs::RenderingAPI::Metal, Internal::DummyImGuiRenderer>,
			Types::EnumToType<Information::Structs::RenderingAPI::Dummy, Internal::DummyImGuiRenderer>
		>;
	public:
		// Init & Destroy
		inline static void Init(const Device& device, const Swapchain& swapchain, const Renderpass& renderpass) { s_Instance.Construct(device, swapchain, renderpass); }
		inline static void Destroy() { s_Instance.Destroy(); }

		// Methods
		inline static void Begin() { s_Instance->Begin(); }
		inline static void End(const CommandList& commandList) { s_Instance->End(commandList); }

	private:
		inline static Nano::Memory::DeferredConstruct<Type, true> s_Instance = {};
	};

}