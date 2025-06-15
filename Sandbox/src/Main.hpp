#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Window.hpp"

#include "NanoGraphics/Renderer/Device.hpp"

#include <Nano/Nano.hpp>

#include <string_view>

using namespace Nano;
using namespace Nano::Graphics;

inline constexpr std::string_view g_VertexShader = R"(
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(location = 0) out vec3 v_Position;
layout(location = 1) out vec2 v_TexCoord;

layout(std140, set = 0, binding = 0) uniform CameraSettings
{
    mat4 View;
    mat4 Projection;
} u_Camera;

void main()
{
    v_Position = a_Position;
    v_TexCoord = a_TexCoord;

    gl_Position = u_Camera.Projection * u_Camera.View * vec4(a_Position, 1.0);
}
)";

inline constexpr std::string_view g_FragmentShader = R"(
#version 460 core

layout(location = 0) out vec4 o_Colour;

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec2 v_TexCoord;

layout (set = 0, binding = 1) uniform texture2D u_Texture;
layout (set = 0, binding = 12) uniform sampler u_Sampler;

void main()
{
	// Combine texture and sampler
    o_Colour = texture(sampler2D(u_Texture, u_Sampler), v_TexCoord);
}
)";

int Main(int argc, char* argv[])
{
	(void)argc; (void)argv;

	{
		// Global pointers
		Window* windowPtr = nullptr;
		Swapchain* swapchainPtr = nullptr;
		Renderpass* renderpassPtr = nullptr;

		// Window Creation
		WindowSpecification windowSpecs = WindowSpecification()
			.SetTitle("First")
			.SetWidthAndHeight(1280, 720)
			.SetFlags(WindowFlags::Default)
			.SetEventCallback([&](Event e) 
			{
				Events::EventHandler handler(e);
				handler.Handle<WindowCloseEvent>([&](WindowCloseEvent&) mutable { windowPtr->Close(); });
				handler.Handle<WindowResizeEvent>([&](WindowResizeEvent& wre) mutable 
				{ 
					swapchainPtr->Resize(wre.GetWidth(), wre.GetHeight()); 
					renderpassPtr->ResizeFramebuffers();
				});
			});
		Window window(windowSpecs);
		windowPtr = &window;

		// Destroy prep
		std::queue<DeviceDestroyFn> destroyQueue = {};
		auto emptyQueue = [&]() { while (!destroyQueue.empty()) { destroyQueue.front()(); destroyQueue.pop(); } };

		// Device Creation
		DeviceSpecification deviceSpecs = DeviceSpecification()
			.SetNativeWindow(windowPtr->GetNativeWindow())
			.SetMessageCallback([](DeviceMessageType msgType, const std::string& message)
			{
				switch (msgType)
				{
				case DeviceMessageType::Error:
					NG_LOG_ERROR("Device Error: {0}", message);
					break;
				case DeviceMessageType::Warn:
					NG_LOG_WARN("Device Warning: {0}", message);
					break;

				default:
					break;
				}
			})
			.SetDestroyCallback([&](DeviceDestroyFn fn)
			{
				destroyQueue.push(fn);
			});
		Device device(deviceSpecs);

		// Swapchain
		SwapchainSpecification swapchainSpecs = SwapchainSpecification()
			.SetWindow(window)
			.SetFormat(Format::BGRA8Unorm)
			//.SetFormat(Format::SBGRA8Unorm)
			.SetColourSpace(ColourSpace::SRGB)
			.SetVSync(false)
			.SetDebugName("Swapchain");
		Swapchain swapchain = device.CreateSwapchain(swapchainSpecs);
		swapchainPtr = &swapchain;

		// Commandlists & Commandpool
		CommandListPool pool = swapchain.AllocateCommandListPool({ "First pool" });
		std::array<CommandList, Information::BackBufferCount> lists = {
			pool.AllocateList({ "First List" }),
			pool.AllocateList({ "Second List" }),
			pool.AllocateList({ "Third List" }),
		};

		// Renderpass & Framebuffers
		RenderpassSpecification renderpassSpecs = RenderpassSpecification()
			.SetBindpoint(PipelineBindpoint::Graphics)
			
			.SetColourImageSpecification(swapchain.GetImage(0).GetSpecification())
			.SetColourLoadOperation(LoadOperation::Clear)
			.SetColourStoreOperation(StoreOperation::Store)
			.SetColourStartState(ResourceState::Unknown)
			.SetColourEndState(ResourceState::Present)

			.SetDebugName("Renderpass");
		Renderpass renderpass = device.CreateRenderpass(renderpassSpecs);
		renderpassPtr = &renderpass;

		for (uint8_t i = 0; i < Information::BackBufferCount; i++)
		{
			std::string debugName = std::format("Framebuffer({0}) for: {1}", i, renderpassSpecs.DebugName);
			FramebufferSpecification framebufferSpecs = FramebufferSpecification()
				.SetColourAttachment(FramebufferAttachment()
					.SetImage(swapchain.GetImage(i)))
				.SetDebugName(debugName);

			(void)renderpass.CreateFramebuffer(framebufferSpecs);
		}

		// ShaderCompiler & Shader
		ShaderCompiler compiler;
		std::vector<char> vertexSPIRV = compiler.CompileToSPIRV(ShaderStage::Vertex, std::string(g_VertexShader), ShadingLanguage::GLSL);
		std::vector<char> fragmentSPIRV = compiler.CompileToSPIRV(ShaderStage::Fragment, std::string(g_FragmentShader), ShadingLanguage::GLSL);

		auto shaders = std::to_array<Shader>({
			device.CreateShader({ ShaderStage::Vertex, vertexSPIRV, "Vertex Shader" }),
			device.CreateShader({ ShaderStage::Fragment, fragmentSPIRV, "Fragment Shader" }),
		});

		// Bindingsets & Layouts
		InputLayout inputLayout = device.CreateInputLayout({ 
			VertexAttributeSpecification()
				.SetBufferIndex(0)
				.SetFormat(Format::RGB32Float)
				.SetSize(VertexAttributeSpecification::AutoSize)
				.SetOffset(VertexAttributeSpecification::AutoOffset)
				.SetDebugName("a_Position"),
			VertexAttributeSpecification()
				.SetBufferIndex(0)
				.SetFormat(Format::RG32Float)
				.SetSize(VertexAttributeSpecification::AutoSize)
				.SetOffset(VertexAttributeSpecification::AutoOffset)
				.SetDebugName("a_TexCoord")
		});

		BindingLayout bindingLayoutSet0 = device.CreateBindingLayout(BindingLayoutSpecification()
			.SetRegisterSpace(0)
			.SetRegisterSpaceIsDescriptorSet(true)

			// Vertex
			.AddItem(BindingLayoutItem()
				.SetSlot(0)
				.SetVisibility(ShaderStage::Vertex)
				.SetType(ResourceType::UniformBuffer)
				.SetDebugName("u_Camera")
			)

			// Fragment
			.AddItem(BindingLayoutItem()
				.SetSlot(1)
				.SetVisibility(ShaderStage::Fragment)
				.SetType(ResourceType::Image)
				.SetDebugName("u_Texture")
			)
			.AddItem(BindingLayoutItem()
				.SetSlot(2)
				.SetVisibility(ShaderStage::Fragment)
				.SetType(ResourceType::Sampler)
				.SetDebugName("u_Sampler")
			)

			//.SetBindingOffsets(VulkanBindingOffsets(0, 0, 0, 0))
		);

		BindingSetPool bindingSetPoolSet0 = device.AllocateBindingSetPool(BindingSetPoolSpecification()
			.SetLayout(bindingLayoutSet0)
			.SetSetAmount(Information::BackBufferCount)
			.SetDebugName("BindingSetPool")
		);

		std::array<BindingSet, Information::BackBufferCount> set0s = {
			bindingSetPoolSet0.CreateBindingSet(),
			bindingSetPoolSet0.CreateBindingSet(),
			bindingSetPoolSet0.CreateBindingSet(),
		};

		// Pipeline
		// TODO: ...

		// Main Loop
		while (window.IsOpen())
		{
			if (window.IsFocused()) [[likely]]
				window.PollEvents();
			else
				window.WaitEvents(1.0); // Note: When the windows is out of focus it only updates every second

			emptyQueue();
			swapchain.AcquireNextImage();

			CommandList& list = lists[swapchain.GetCurrentFrame()];
			BindingSet& set0 = set0s[swapchain.GetCurrentFrame()];
			
			{
				list.ResetAndOpen();
				{ 
					// Graphics
					GraphicsState state = GraphicsState()
						.SetRenderpass(renderpass)
						.SetViewport(Viewport(static_cast<float>(window.GetSize().x), static_cast<float>(window.GetSize().y)))
						.SetScissor(ScissorRect(Viewport(static_cast<float>(window.GetSize().x), static_cast<float>(window.GetSize().y))))
						.SetColourClear({ (static_cast<float>(window.GetInput().GetCursorPosition().x) / static_cast<float>(window.GetSize().x)), (static_cast<float>(window.GetInput().GetCursorPosition().y) / static_cast<float>(window.GetSize().y)), 0.0f, 1.0f })
						.AddBindingSet(0, set0);
					list.SetGraphicsState(state);

					// Rendering
					// ...
				}
				list.Close();

				CommandListSubmitArgs args = CommandListSubmitArgs()
					.SetQueue(CommandQueue::Graphics)
					.SetWaitForSwapchainImage(true)
					.SetOnFinishMakeSwapchainPresentable(true);
				list.Submit(args);
			}
			swapchain.Present();
		}

		device.FreeBindingSetPool(bindingSetPoolSet0);

		device.DestroyBindingLayout(bindingLayoutSet0);

		for (auto& shader : shaders)
			device.DestroyShader(shader);

		device.DestroyRenderpass(renderpass);

		swapchain.FreePool(pool);
		device.DestroySwapchain(swapchain);

		device.Wait();
		emptyQueue();
	}

	return 0;
}