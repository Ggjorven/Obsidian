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

//layout(std140, set = 0, binding = 0) uniform CameraSettings
//{
//    mat4 View;
//    mat4 Projection;
//} u_Camera;

void main()
{
    v_Position = a_Position;
    v_TexCoord = a_TexCoord;

    //gl_Position = u_Camera.Projection * u_Camera.View * vec4(a_Position, 1.0);
    gl_Position = vec4(a_Position, 1.0);
}
)";

inline constexpr std::string_view g_FragmentShader = R"(
#version 460 core

layout(location = 0) out vec4 o_Colour;

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec2 v_TexCoord;

layout (set = 0, binding = 1) uniform texture2D u_Texture;
layout (set = 0, binding = 2) uniform sampler u_Sampler;

void main()
{
	// Combine texture and sampler
    o_Colour = texture(sampler2D(u_Texture, u_Sampler), v_TexCoord);
	//o_Colour = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

inline constexpr auto g_VertexData = std::to_array<float>({
	// Positions				// UVs
	-0.5f, -0.5f, 0.0f,			0.0f, 0.0f,
	0.5f,  -0.5f, 0.0f,			1.0f, 0.0f,
	0.5f,  0.5f,  0.0f,			1.0f, 1.0f,
	-0.5f, 0.5f,  0.0f,			0.0f, 1.0f
});

inline constexpr auto g_IndexData = std::to_array<uint32_t>({
	0u, 1u, 2u,
	2u, 3u, 0u
});

int Main(int argc, char* argv[])
{
	(void)argc; (void)argv;

	{
		// Global pointers
		Window* windowPtr = nullptr;
		Swapchain* swapchainPtr = nullptr;
		Renderpass* renderpassPtr = nullptr;

		// Window Creation
		Window window(WindowSpecification()
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
			})
		);
		windowPtr = &window;

		// Destroy prep
		std::queue<DeviceDestroyFn> destroyQueue = {};
		auto emptyQueue = [&]() { while (!destroyQueue.empty()) { destroyQueue.front()(); destroyQueue.pop(); } };

		// Device Creation
		Device device(DeviceSpecification()
			.SetNativeWindow(windowPtr->GetNativeWindow())
			.SetMessageCallback([](DeviceMessageType msgType, const std::string& message)
			{
				switch (msgType)
				{
				case DeviceMessageType::Warn:
					NG_LOG_WARN("Device Warning: {0}", message);
					break;
				case DeviceMessageType::Error:
					NG_LOG_ERROR("Device Error: {0}", message);
					break;

				default:
					break;
				}
			})
			.SetDestroyCallback([&](DeviceDestroyFn fn)
			{
				destroyQueue.push(fn);
			})
		);

		// Swapchain
		Swapchain swapchain = device.CreateSwapchain(SwapchainSpecification()
			.SetWindow(window)
			.SetFormat(Format::BGRA8Unorm)
			//.SetFormat(Format::SBGRA8Unorm)
			.SetColourSpace(ColourSpace::SRGB)
			.SetVSync(false)
			.SetDebugName("Swapchain")
		);
		swapchainPtr = &swapchain;

		// Commandlists & Commandpool
		CommandListPool pool = swapchain.AllocateCommandListPool({ "First pool" });
		std::array<CommandList, Information::BackBufferCount> lists = {
			pool.AllocateList({ "First List" }),
			pool.AllocateList({ "Second List" }),
			pool.AllocateList({ "Third List" }),
		};

		// Renderpass & Framebuffers
		Renderpass renderpass = device.CreateRenderpass(RenderpassSpecification()
			.SetBindpoint(PipelineBindpoint::Graphics)

			.SetColourImageSpecification(swapchain.GetImage(0).GetSpecification())
			.SetColourLoadOperation(LoadOperation::Clear)
			.SetColourStoreOperation(StoreOperation::Store)
			.SetColourStartState(ResourceState::Unknown)
			.SetColourEndState(ResourceState::Present)

			.SetDebugName("Renderpass")
		);
		renderpassPtr = &renderpass;

		for (uint8_t i = 0; i < Information::BackBufferCount; i++)
		{
			std::string debugName = std::format("Framebuffer({0}) for: {1}", i, renderpass.GetSpecification().DebugName);
			(void)renderpass.CreateFramebuffer(FramebufferSpecification()
				.SetColourAttachment(FramebufferAttachment()
					.SetImage(swapchain.GetImage(i)))
				.SetDebugName(debugName)
			);
		}

		// ShaderCompiler & Shader
		ShaderCompiler compiler;
		std::vector<char> vertexSPIRV = compiler.CompileToSPIRV(ShaderStage::Vertex, std::string(g_VertexShader), ShadingLanguage::GLSL);
		std::vector<char> fragmentSPIRV = compiler.CompileToSPIRV(ShaderStage::Fragment, std::string(g_FragmentShader), ShadingLanguage::GLSL);

		auto shaders = std::to_array<Shader>({
			device.CreateShader({ ShaderStage::Vertex, "main", vertexSPIRV, "Vertex Shader" }),
			device.CreateShader({ ShaderStage::Fragment, "main", fragmentSPIRV, "Fragment Shader" }),
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

			/*
			// Vertex
			.AddItem(BindingLayoutItem()
				.SetSlot(0)
				.SetVisibility(ShaderStage::Vertex)
				.SetType(ResourceType::UniformBuffer)
				.SetDebugName("u_Camera")
			)

			// Fragment
			*/
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

			.SetBindingOffsets(VulkanBindingOffsets(0, 0, 0, 0))
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
		GraphicsPipeline pipeline = device.CreateGraphicsPipeline(GraphicsPipelineSpecification()
			.SetPrimitiveType(PrimitiveType::TriangleList)
			.SetInputLayout(inputLayout)
			.SetVertexShader(shaders[0])
			.SetFragmentShader(shaders[1])

			.SetRenderState(RenderState()
				.SetRasterState(RasterState()
					.SetCullingMode(RasterCullingMode::None)
					// TODO: ...
				)
				.SetBlendState(BlendState()
					.SetRenderTarget(BlendState::RenderTarget()
						.SetBlendEnable(true)
						// TODO: ...
					)
					// TODO: ...
				)
				.SetDepthStencilState(DepthStencilState()
					.SetDepthTestEnable(false)
					// TODO: ...
				)
			)

			.SetRenderpass(renderpass)
			.AddBindingLayout(bindingLayoutSet0)
		);

		// Buffers & Image initialization
		CommandList initCommand = pool.AllocateList(CommandListSpecification()
			.SetDebugName("InitCommand")
		);
		initCommand.Open();

		Buffer stagingBuffer = device.CreateBuffer(BufferSpecification()
			.SetSize(sizeof(g_VertexData) + sizeof(g_IndexData)) // std::max(sizeof(g_VertexData), sizeof(g_IndexData))
			.SetCPUAccess(CpuAccessMode::Write)
		);
		initCommand.StartTracking(stagingBuffer, ResourceState::Unknown);

		void* bufferMemory;
		device.MapBuffer(stagingBuffer, bufferMemory);

		Buffer vertexBuffer = device.CreateBuffer(BufferSpecification()
			.SetSize(sizeof(g_VertexData))
			.SetIsVertexBuffer(true)
			.SetDebugName("Vertexbuffer")
		);
		initCommand.StartTracking(vertexBuffer, ResourceState::Unknown);
		std::memcpy(bufferMemory, static_cast<const void*>(g_VertexData.data()), sizeof(g_VertexData));
		initCommand.CopyBuffer(vertexBuffer, stagingBuffer, sizeof(g_VertexData), 0, 0);

		Buffer indexBuffer = device.CreateBuffer(BufferSpecification()
			.SetSize(sizeof(g_IndexData))
			.SetFormat(Format::R32UInt)
			.SetIsIndexBuffer(true)
			.SetDebugName("Indexbuffer")
		);
		initCommand.StartTracking(indexBuffer, ResourceState::Unknown);
		std::memcpy(static_cast<uint8_t*>(bufferMemory) + sizeof(g_VertexData), g_IndexData.data(), sizeof(g_IndexData));
		initCommand.CopyBuffer(indexBuffer, stagingBuffer, sizeof(g_IndexData), sizeof(g_VertexData), 0);

		StagingImage stagingImage = device.CreateStagingImage(ImageSpecification()
			.SetImageFormat(Format::RGBA8Unorm)
			.SetImageDimension(ImageDimension::Image2D)
			.SetWidthAndHeight(1, 1), 
			CpuAccessMode::Write
		);
		initCommand.StartTracking(stagingImage, ResourceState::Unknown);
		
		void* imageMemory;
		device.MapStagingImage(stagingImage, imageMemory);

		Image image = device.CreateImage(ImageSpecification()
			.SetImageFormat(Format::RGBA8Unorm)
			.SetImageDimension(ImageDimension::Image2D)
			.SetPermanentState(ResourceState::ShaderResource)
			.SetWidthAndHeight(1, 1)
			.SetIsShaderResource(true)
			.SetMipLevels(1)
		);
		initCommand.StartTracking(image, ImageSubresourceSpecification(0, 1, 0, 1), ResourceState::Unknown);
		uint32_t imageColour = 0xFF00FF00;
		std::memcpy(static_cast<uint8_t*>(imageMemory), &imageColour, sizeof(imageColour));
		initCommand.CopyImage(image, ImageSliceSpecification(), stagingImage, ImageSliceSpecification());

		Sampler sampler = device.CreateSampler(SamplerSpecification());

		device.UnmapStagingImage(stagingImage);
		device.UnmapBuffer(stagingBuffer);

		initCommand.Close();
		initCommand.Submit(CommandListSubmitArgs().SetQueue(CommandQueue::Graphics));

		initCommand.WaitTillComplete();

		device.DestroyBuffer(stagingBuffer);
		device.DestroyStagingImage(stagingImage);

		// Upload to bindingsets
		for (auto& bindingSet : set0s)
		{
			bindingSet.Upload(image, ImageSubresourceSpecification(0, 1, 0, 1), ResourceType::Image, 1, 0);
			bindingSet.Upload(sampler, ResourceType::Sampler, 2, 0);
		}

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
						.SetPipeline(pipeline)
						.SetRenderpass(renderpass)
						.SetViewport(Viewport(static_cast<float>(window.GetSize().x), static_cast<float>(window.GetSize().y)))
						.SetScissor(ScissorRect(Viewport(static_cast<float>(window.GetSize().x), static_cast<float>(window.GetSize().y))))
						.SetColourClear({ (static_cast<float>(window.GetInput().GetCursorPosition().x) / static_cast<float>(window.GetSize().x)), (static_cast<float>(window.GetInput().GetCursorPosition().y) / static_cast<float>(window.GetSize().y)), 0.0f, 1.0f })
						.AddBindingSet(0, set0);
					list.SetGraphicsState(state);

					// Rendering
					list.BindVertexBuffer(vertexBuffer);
					list.BindIndexBuffer(indexBuffer);

					list.DrawIndexed(DrawArguments()
						.SetVertexCount((sizeof(g_IndexData) / sizeof(g_IndexData[0])))
						.SetInstanceCount(1)
					);
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

		device.DestroySampler(sampler);
		device.DestroyImage(image);

		device.DestroyBuffer(indexBuffer);
		device.DestroyBuffer(vertexBuffer);

		device.DestroyGraphicsPipeline(pipeline);

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