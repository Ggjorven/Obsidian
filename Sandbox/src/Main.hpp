#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Window.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/ImGuiRenderer.hpp"

#include "NanoGraphics/Maths/Structs.hpp"

#include <Nano/Nano.hpp>

#include "Utilities/Camera.hpp"

#include <string_view>

using namespace Nano::Graphics;

#if 1
inline constexpr ShadingLanguage g_ShadingLanguage = ShadingLanguage::GLSL;

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
    //gl_Position = vec4(a_Position, 1.0);
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
#else
inline constexpr ShadingLanguage g_ShadingLanguage = ShadingLanguage::HLSL;

inline constexpr std::string_view g_VertexShader = R"(
struct VSInput
{
    float3 Position : POSITION0;
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput
{
    float2 TexCoord : TEXCOORD0;
    float3 Position : TEXCOORD1;
    float4 SV_Position : SV_POSITION;
};

cbuffer CameraSettings : register(b0, space0)
{
    float4x4 View;
    float4x4 Projection;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.Position = input.Position;
    output.TexCoord = input.TexCoord;

    output.SV_Position = mul(Projection, mul(View, float4(input.Position, 1.0)));
    //output.SV_Position = float4(input.Position, 1.0);
    return output;
}
)";

inline constexpr std::string_view g_FragmentShader = R"(
Texture2D u_Texture : register(t1, space0);
SamplerState u_Sampler : register(s2, space0);

struct PSInput
{
    float2 TexCoord : TEXCOORD0;
    float3 Position : TEXCOORD1;
};

float4 main(PSInput input) : SV_Target
{
    return u_Texture.Sample(u_Sampler, input.TexCoord);
    // return float4(1.0, 0.0, 0.0, 1.0);
}
)";
#endif

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

class Application
{
public:
	// Constructor & Destructor
	Application()
	{
		// Window
		m_Window.Construct(WindowSpecification()
			.SetTitle("First")
			.SetWidthAndHeight(1280, 720)
			.SetFlags(WindowFlags::Resizable | WindowFlags::Decorated | WindowFlags::Visible | WindowFlags::Focused | WindowFlags::FocusOnShow)
			.SetEventCallback([this](Event e) { OnEvent(e); })
		);

		// Device
		m_Device.Construct(DeviceSpecification()
			.SetNativeWindow(m_Window->GetNativeWindow())
			.SetMessageCallback([this](DeviceMessageType msgType, const std::string& message) { OnDeviceMessage(msgType, message); })
			.SetDestroyCallback([this](DeviceDestroyFn fn) { m_DestroyQueue.push(fn); })
		);

		// Swapchain
		m_Swapchain.Construct(m_Device.Get(), SwapchainSpecification()
			.SetWindow(m_Window.Get())
			.SetFormat(Format::BGRA8Unorm)
			.SetColourSpace(ColourSpace::SRGB)
#if defined(NG_PLATFORM_APPLE)
			.SetVSync(true) // Note: Vulkan via MoltenVK without VSync causes bad screen tearing.
#else
			.SetVSync(false)
#endif
			.SetDebugName("Swapchain")
		);

		// Commandlists
		for (size_t i = 0; i < m_RenderpassLists.size(); i++)
		{
			m_CommandPools[i].Construct(m_Swapchain.Get(), CommandListPoolSpecification().SetDebugName("CommandPool"));
		}
		for (size_t i = 0; i < m_RenderpassLists.size(); i++)
		{
			std::string debugName = std::format("RenderpassList({0}) for: {1}", i, m_CommandPools[i]->GetSpecification().DebugName);
			m_RenderpassLists[i].Construct(m_CommandPools[i].Get(), CommandListSpecification().SetDebugName(debugName));
		}
		for (size_t i = 0; i < m_ImGuiLists.size(); i++)
		{
			std::string debugName = std::format("ImguiList({0}) for: {1}", i, m_CommandPools[i]->GetSpecification().DebugName);
			m_ImGuiLists[i].Construct(m_CommandPools[i].Get(), CommandListSpecification().SetDebugName(debugName));
		}

		// Renderpass & Framebuffers
		m_Renderpass.Construct(m_Device.Get(), RenderpassSpecification()
			.SetBindpoint(PipelineBindpoint::Graphics)

			.SetColourImageSpecification(m_Swapchain->GetImage(0).GetSpecification())
			.SetColourLoadOperation(LoadOperation::Clear)
			.SetColourStoreOperation(StoreOperation::Store)
			.SetColourStartState(ResourceState::Present)
			.SetColourRenderingState(ResourceState::RenderTarget)
			.SetColourEndState(ResourceState::RenderTarget)

			.SetDebugName("Renderpass")
		);

		for (size_t i = 0; i < m_Swapchain->GetImageCount(); i++)
		{
			std::string debugName = std::format("Framebuffer({0}) for: {1}", i, m_Renderpass->GetSpecification().DebugName);
			(void)m_Renderpass->CreateFramebuffer(FramebufferSpecification()
				.SetColourAttachment(FramebufferAttachment()
					.SetImage(m_Swapchain->GetImage(static_cast<uint8_t>(i)))
				)
				.SetDebugName(debugName)
			);
		}

		// Imgui Renderpass & Framebuffers
		m_ImguiPass.Construct(m_Device.Get(), RenderpassSpecification()
			.SetBindpoint(PipelineBindpoint::Graphics)

			.SetColourImageSpecification(m_Swapchain->GetImage(0).GetSpecification())
			.SetColourLoadOperation(LoadOperation::Load)
			.SetColourStoreOperation(StoreOperation::Store)
			.SetColourStartState(ResourceState::RenderTarget)
			.SetColourRenderingState(ResourceState::RenderTarget)
			.SetColourEndState(ResourceState::Present)

			.SetDebugName("ImguiPass")
		);

		for (size_t i = 0; i < m_Swapchain->GetImageCount(); i++)
		{
			std::string debugName = std::format("Framebuffer({0}) for: {1}", i, m_ImguiPass->GetSpecification().DebugName);
			(void)m_ImguiPass->CreateFramebuffer(FramebufferSpecification()
				.SetColourAttachment(FramebufferAttachment()
					.SetImage(m_Swapchain->GetImage(static_cast<uint8_t>(i)))
				)
				.SetDebugName(debugName)
			);
		}

		ImGuiRenderer::Init(m_Device.Get(), m_Swapchain.Get(), m_ImguiPass.Get());

		// Shaders
		ShaderCompiler compiler;
		std::vector<uint32_t> vertexSPIRV = compiler.CompileToSPIRV(ShaderStage::Vertex, std::string(g_VertexShader), "main", g_ShadingLanguage);
		std::vector<uint32_t> fragmentSPIRV = compiler.CompileToSPIRV(ShaderStage::Fragment, std::string(g_FragmentShader), "main", g_ShadingLanguage);

		Shader vertexShader = m_Device->CreateShader({ ShaderStage::Vertex, "main", vertexSPIRV, "Vertex Shader" });
		Shader fragmentShader = m_Device->CreateShader({ ShaderStage::Fragment, "main", fragmentSPIRV, "Fragment Shader" });

		// Layouts
		m_InputLayout.Construct(m_Device.Get(), std::initializer_list<VertexAttributeSpecification>({
			VertexAttributeSpecification()
				.SetBufferIndex(0)
				.SetLocation(0)
				.SetFormat(Format::RGB32Float)
				.SetSize(VertexAttributeSpecification::AutoSize)
				.SetOffset(VertexAttributeSpecification::AutoOffset)
				.SetDebugName("a_Position"),
			VertexAttributeSpecification()
				.SetBufferIndex(0)
				.SetLocation(1)
				.SetFormat(Format::RG32Float)
				.SetSize(VertexAttributeSpecification::AutoSize)
				.SetOffset(VertexAttributeSpecification::AutoOffset)
				.SetDebugName("a_TexCoord")
		}));

		m_BindingLayoutSet0.Construct(m_Device.Get(), BindingLayoutSpecification()
			.SetRegisterSpace(0)

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

			.SetDebugName("Layout for: Set0")
		);

		// BindingPool & Sets
		m_BindingSetPool0.Construct(m_Device.Get(), BindingSetPoolSpecification()
			.SetLayout(m_BindingLayoutSet0.Get())
			.SetSetAmount(Information::FramesInFlight)
			.SetDebugName("BindingSetPool0")
		);
		for (size_t i = 0; i < m_Set0s.size(); i++)
		{
			m_Set0s[i].Construct(m_BindingSetPool0.Get(), BindingSetSpecification());
		}

		// Pipeline
		m_Pipeline.Construct(m_Device.Get(), GraphicsPipelineSpecification()
			.SetPrimitiveType(PrimitiveType::TriangleList)
			.SetInputLayout(m_InputLayout.Get())
			.SetVertexShader(vertexShader)
			.SetFragmentShader(fragmentShader)

			.SetRenderState(RenderState()
				.SetRasterState(RasterState()
					.SetFillMode(RasterFillMode::Fill)
					.SetCullingMode(RasterCullingMode::None)
					.SetFrontCounterClockwise(true)
					.SetDepthBias(0)
					.SetDepthBiasClamp(0.0f)
				)
				.SetBlendState(BlendState()
					.SetRenderTarget(BlendState::RenderTarget()
						.SetBlendEnable(true)
						.SetSrcBlend(BlendFactor::One)
						.SetDstBlend(BlendFactor::Zero)
						.SetBlendOperation(BlendOperation::Add)
						.SetSrcBlendAlpha(BlendFactor::One)
						.SetDstBlendAlpha(BlendFactor::Zero)
						.SetBlendOpAlpha(BlendOperation::Add)
						.SetColourWriteMask(ColourMask::All)
					)
					.SetAlphaToCoverageEnable(false)
				)
				.SetDepthStencilState(DepthStencilState()
					.SetDepthTestEnable(false)	// Note: Enable for depth
					.SetDepthWriteEnable(false)	// Note: Enable for depth
					.SetDepthFunc(ComparisonFunc::Less)
					.SetStencilEnable(false)
				)
			)

			.SetRenderpass(m_Renderpass.Get())
			.AddBindingLayout(m_BindingLayoutSet0.Get())
			.SetDebugName("GraphicsPipeline")
		);

		m_Device->DestroyShader(vertexShader);
		m_Device->DestroyShader(fragmentShader);

		// Initialization
		{
			CommandList initCommand = m_CommandPools[0]->AllocateList(CommandListSpecification().SetDebugName("InitCommand"));
			initCommand.Open();

			Buffer stagingBuffer = m_Device->CreateBuffer(BufferSpecification()
				.SetSize(sizeof(g_VertexData) + sizeof(g_IndexData))
				.SetCPUAccess(CpuAccessMode::Write)
			);
			m_Device->StartTracking(stagingBuffer, ResourceState::Unknown);

			void* bufferMemory;
			m_Device->MapBuffer(stagingBuffer, bufferMemory);

			// Vertex
			{
				m_VertexBuffer.Construct(m_Device.Get(), BufferSpecification()
					.SetSize(sizeof(g_VertexData))
					.SetIsVertexBuffer(true)
					.SetDebugName("Vertexbuffer")
				);
				m_Device->StartTracking(m_VertexBuffer.Get(), ResourceState::Unknown);
				if (bufferMemory) 
					std::memcpy(bufferMemory, static_cast<const void*>(g_VertexData.data()), sizeof(g_VertexData));
				
				initCommand.CopyBuffer(m_VertexBuffer.Get(), stagingBuffer, sizeof(g_VertexData), 0, 0);
			}

			// Index
			{
				m_IndexBuffer.Construct(m_Device.Get(), BufferSpecification()
					.SetSize(sizeof(g_IndexData))
					.SetFormat(Format::R32UInt)
					.SetIsIndexBuffer(true)
					.SetDebugName("Indexbuffer")
				);
				m_Device->StartTracking(m_IndexBuffer.Get(), ResourceState::Unknown);
				if (bufferMemory) 
					std::memcpy(static_cast<uint8_t*>(bufferMemory) + sizeof(g_VertexData), g_IndexData.data(), sizeof(g_IndexData));
				
				initCommand.CopyBuffer(m_IndexBuffer.Get(), stagingBuffer, sizeof(g_IndexData), sizeof(g_VertexData), 0);
			}

			// Uniform
			{
				m_UniformBuffer.Construct(m_Device.Get(), BufferSpecification()
					.SetSize(sizeof(Camera))
					.SetIsUniformBuffer(true)
					.SetCPUAccess(CpuAccessMode::Write)
					.SetDebugName("Camera Uniform")
				);
				m_Device->StartTracking(m_UniformBuffer.Get(), ResourceState::Unknown);
				
				m_Device->MapBuffer(m_UniformBuffer.Get(), m_UniformMemory);

				m_Camera.Construct(m_Window.Get());

				if (m_UniformMemory)
					std::memcpy(static_cast<uint8_t*>(m_UniformMemory), &m_Camera->GetCamera(), sizeof(CameraData));
			}

			StagingImage stagingImage = m_Device->CreateStagingImage(ImageSpecification()
				.SetImageFormat(Format::RGBA8Unorm)
				.SetImageDimension(ImageDimension::Image2D)
				.SetWidthAndHeight(1, 1),
				CpuAccessMode::Write
			);
			m_Device->StartTracking(stagingImage, ResourceState::Unknown);

			uint32_t imageColour = 0xFFFFFF00;
			m_Device->WriteImage(stagingImage, ImageSliceSpecification(), &imageColour, sizeof(uint32_t));

			// Image
			{
				m_Image.Construct(m_Device.Get(), ImageSpecification()
					.SetImageFormat(Format::RGBA8Unorm)
					.SetImageDimension(ImageDimension::Image2D)
					.SetPermanentState(ResourceState::ShaderResource)
					.SetWidthAndHeight(1, 1)
					.SetIsShaderResource(true)
					.SetMipLevels(1)
					.SetDebugName("Temp image")
				);
				m_Device->StartTracking(m_Image.Get(), ImageSubresourceSpecification(0, 1, 0, 1), ResourceState::Unknown);
				
				initCommand.CopyImage(m_Image.Get(), ImageSliceSpecification(), stagingImage, ImageSliceSpecification());
			}

			// Sampler
			std::string samplerName = std::format("Sampler for: {0}", m_Image->GetSpecification().DebugName);
			m_Sampler.Construct(m_Device.Get(), SamplerSpecification().SetDebugName(samplerName));

			// Destroy
			m_Device->UnmapBuffer(stagingBuffer);

			initCommand.Close();
			initCommand.Submit(CommandListSubmitArgs());

			initCommand.WaitTillComplete();

			m_Device->DestroyBuffer(stagingBuffer);
			m_Device->DestroyStagingImage(stagingImage);

			// Upload
			for (auto& bindingSet : m_Set0s)
			{
				bindingSet->SetItem(0, m_UniformBuffer.Get(), BufferRange(BufferRange::FullSize, 0), 0);
				bindingSet->SetItem(1, m_Image.Get(), ImageSubresourceSpecification(0, 1, 0, 1), 0);
				bindingSet->SetItem(2, m_Sampler.Get(), 0);
			}
		}
	}
	~Application()
	{
		ImGuiRenderer::Destroy();

		m_Device->UnmapBuffer(m_UniformBuffer.Get());

		m_Device->DestroySampler(m_Sampler.Get());
		m_Device->DestroyImage(m_Image.Get());

		m_Device->DestroyBuffer(m_UniformBuffer.Get());
		m_Device->DestroyBuffer(m_IndexBuffer.Get());
		m_Device->DestroyBuffer(m_VertexBuffer.Get());

		m_Device->DestroyGraphicsPipeline(m_Pipeline.Get());

		m_Device->FreeBindingSetPool(m_BindingSetPool0.Get());

		m_Device->DestroyBindingLayout(m_BindingLayoutSet0.Get());

		m_Device->DestroyRenderpass(m_ImguiPass.Get());
		m_Device->DestroyRenderpass(m_Renderpass.Get());

		for (size_t i = 0; i < m_CommandPools.size(); i++)
		{
			m_Swapchain->FreePool(m_CommandPools[i].Get());
		}
		m_Device->DestroySwapchain(m_Swapchain.Get());

		m_Device->Wait();
		FreeQueue();
	}

	// Methods
	void Run()
	{
		double lastTime = 0.0;

		while (m_Window->IsOpen())
		{
			if (m_Window->IsFocused()) [[likely]]
				m_Window->PollEvents();
			else
				m_Window->WaitEvents(1.0); // Note: When the windows is out of focus it only updates every second

			FreeQueue();

			double time = m_Window->GetWindowTime(); // Note: We use m_Window->GetWindowTime() instead of Nano's dedicated timer class because steadyclock on MacOS is very weird and unstable.
			Update(static_cast<float>(time - lastTime));
			lastTime = time;

			m_Swapchain->AcquireNextImage();
			{
				m_CommandPools[m_Swapchain->GetCurrentFrame()]->Reset();
				CommandList& renderpassList = m_RenderpassLists[m_Swapchain->GetCurrentFrame()].Get();
				CommandList& imguiList = m_ImGuiLists[m_Swapchain->GetCurrentFrame()].Get();
				BindingSet& set0 = m_Set0s[m_Swapchain->GetCurrentFrame()].Get();

				// Main pass
				{
					renderpassList.Open();

					// Graphics
					renderpassList.StartRenderpass(RenderpassStartArgs()
						.SetRenderpass(m_Renderpass.Get())

						.SetViewport(Viewport(static_cast<float>(m_Window->GetSize().x), static_cast<float>(m_Window->GetSize().y)))
						.SetScissor(ScissorRect(Viewport(static_cast<float>(m_Window->GetSize().x), static_cast<float>(m_Window->GetSize().y))))

						//.SetColourClear({ 
						//	(m_Swapchain->GetCurrentFrame() == 0) ? 1.0f : 0.0f,
						//	(m_Swapchain->GetCurrentFrame() == 1) ? 1.0f : 0.0f,
						//	(m_Swapchain->GetCurrentFrame() == 2) ? 1.0f : 0.0f,
						//	1.0f 
						//})

						.SetColourClear({ 0.0f, 0.0f, 0.0f, 1.0f })
					);

					// Rendering
					renderpassList.BindPipeline(m_Pipeline.Get());

					renderpassList.BindVertexBuffer(m_VertexBuffer.Get());
					renderpassList.BindIndexBuffer(m_IndexBuffer.Get());

					renderpassList.BindBindingSet(set0);

					renderpassList.DrawIndexed(DrawArguments()
						.SetVertexCount((sizeof(g_IndexData) / sizeof(g_IndexData[0])))
						.SetInstanceCount(1)
					);

					renderpassList.EndRenderpass(RenderpassEndArgs().SetRenderpass(m_Renderpass.Get()));
					renderpassList.Close();

					// Submission
					renderpassList.Submit(CommandListSubmitArgs()
						.SetWaitForSwapchainImage(true)
						.SetOnFinishMakeSwapchainPresentable(false)
					);
				}

				// Imgui pass
				{
					imguiList.Open();

					// Imgui
					imguiList.StartRenderpass(RenderpassStartArgs()
						.SetRenderpass(m_ImguiPass.Get())

						.SetViewport(Viewport(static_cast<float>(m_Window->GetSize().x), static_cast<float>(m_Window->GetSize().y)))
						.SetScissor(ScissorRect(Viewport(static_cast<float>(m_Window->GetSize().x), static_cast<float>(m_Window->GetSize().y))))
					);

					// Rendering
					ImGuiRenderer::Begin();
					
					ImGui::ShowMetricsWindow();
					
					ImGuiRenderer::End(imguiList);

					imguiList.EndRenderpass(RenderpassEndArgs().SetRenderpass(m_ImguiPass.Get()));
					imguiList.Close();

					imguiList.Submit(CommandListSubmitArgs()
						.SetWaitOnLists({ &renderpassList })
						.SetWaitForSwapchainImage(false)
						.SetOnFinishMakeSwapchainPresentable(true)
					);
				}
			}
			m_Swapchain->Present();
		}
	}

private:
	// Private methods
	void OnEvent(Event& e)
	{
		Nano::Events::EventHandler handler(e);
		handler.Handle<WindowCloseEvent>([&](WindowCloseEvent&) mutable { m_Window->Close(); });
		handler.Handle<WindowResizeEvent>([&](WindowResizeEvent& wre) mutable
		{
			m_Swapchain->Resize(wre.GetWidth(), wre.GetHeight());
			m_Renderpass->ResizeFramebuffers();
		});

		m_Camera->OnEvent(e);
	}

	void OnDeviceMessage(DeviceMessageType msgType, const std::string& message)
	{
		switch (msgType)
		{
		case DeviceMessageType::Warn:
		{
			NG_LOG_WARN("Device Warning: {0}", message);
			break;
		}
		case DeviceMessageType::Error:
		{
			NG_LOG_ERROR("Device Error: {0}", message);
			break;
		}

		default:
			break;
		}
	}

	void FreeQueue()
	{
		while (!m_DestroyQueue.empty())
		{
			m_DestroyQueue.front()();
			m_DestroyQueue.pop();
		}
	}

	void Update(float deltaTime)
	{
		m_Camera->OnUpdate(deltaTime);

		if (m_UniformMemory)
			std::memcpy(static_cast<uint8_t*>(m_UniformMemory), &m_Camera->GetCamera(), sizeof(CameraData));
	}

private:
	Nano::Memory::DeferredConstruct<Window> m_Window = {};

	Nano::Memory::DeferredConstruct<Device> m_Device = {};
	Nano::Memory::DeferredConstruct<Swapchain> m_Swapchain = {};

	std::array<Nano::Memory::DeferredConstruct<CommandListPool>, Information::FramesInFlight> m_CommandPools = {};
	std::array<Nano::Memory::DeferredConstruct<CommandList>, Information::FramesInFlight> m_RenderpassLists = {};
	std::array<Nano::Memory::DeferredConstruct<CommandList>, Information::FramesInFlight> m_ImGuiLists = {};

	Nano::Memory::DeferredConstruct<Renderpass> m_Renderpass = {};
	Nano::Memory::DeferredConstruct<Renderpass> m_ImguiPass = {};

	Nano::Memory::DeferredConstruct<InputLayout> m_InputLayout = {};
	Nano::Memory::DeferredConstruct<BindingLayout> m_BindingLayoutSet0 = {};

	Nano::Memory::DeferredConstruct<BindingSetPool> m_BindingSetPool0 = {};
	std::array<Nano::Memory::DeferredConstruct<BindingSet>, Information::FramesInFlight> m_Set0s = {};
	Nano::Memory::DeferredConstruct<GraphicsPipeline> m_Pipeline = {};

	Nano::Memory::DeferredConstruct<Buffer> m_VertexBuffer = {};
	Nano::Memory::DeferredConstruct<Buffer> m_IndexBuffer = {};
	
	void* m_UniformMemory;
	Nano::Memory::DeferredConstruct<Buffer> m_UniformBuffer = {};
	Nano::Memory::DeferredConstruct<Camera> m_Camera = {};

	Nano::Memory::DeferredConstruct<Image> m_Image = {};
	Nano::Memory::DeferredConstruct<Sampler> m_Sampler = {};

	std::queue<DeviceDestroyFn> m_DestroyQueue = {};
};

int Main(int argc, char* argv[])
{
	(void)argc; (void)argv;

	Application app;
	app.Run();
	return 0;
}             