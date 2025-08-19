#include "Tests/TestBase.hpp"
#include "Common/Camera2D.hpp"

#include <Obsidian/Maths/Functions.hpp>

////////////////////////////////////////////////////////////////////////////////////
// Shaders
////////////////////////////////////////////////////////////////////////////////////
#if 1
inline constexpr ShadingLanguage g_ShadingLanguage = ShadingLanguage::GLSL;

inline constexpr std::string_view g_VertexShader = R"(
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout(location = 0) out vec3 v_Position;
layout(location = 1) out vec2 v_TexCoord;

layout(push_constant) uniform Camera2DSettings // set = 0, binding = 0
{
    mat4 View;
    mat4 Projection;
} u_Camera2D;

layout(std140, set = 0, binding = 1) uniform ModelSettings
{
    mat4 Model;
} u_Model;

void main()
{
    v_Position = a_Position;
    v_TexCoord = a_TexCoord;

    gl_Position = u_Camera2D.Projection * u_Camera2D.View * u_Model.Model * vec4(a_Position, 1.0);
}
)";

inline constexpr std::string_view g_FragmentShader = R"(
#version 460 core

layout(location = 0) out vec4 o_Colour;

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec2 v_TexCoord;

void main()
{
	o_Colour = vec4(v_TexCoord.x, v_TexCoord.y, 0.0, 1.0);
}
)";
#else
#error HLSL not done
#endif

////////////////////////////////////////////////////////////////////////////////////
// Vertex data
////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////
// ModelData
////////////////////////////////////////////////////////////////////////////////////
struct ModelData
{
public:
	Maths::Mat4<float> Matrix;
};

////////////////////////////////////////////////////////////////////////////////////
// Test
////////////////////////////////////////////////////////////////////////////////////
class DynamicUniformBuffer : public TestBase
{
public:
	// Constructor & Destructor
	DynamicUniformBuffer()
		: TestBase(1280, 720, "DynamicUniformBuffer", [this](Event e) { OnEvent(e); }, [this](DeviceMessageType type, const std::string& msg) { OnDeviceMessage(type, msg); })
	{
		// Commandpools & Commandlists
		for (auto& pool : m_CommandPools)
			pool.Construct(m_Swapchain.Get(), CommandListPoolSpecification()
				.SetQueue(CommandQueue::Graphics)
				.SetDebugName("CommandPool")
			);

		for (size_t i = 0; i < m_CommandLists.size(); i++)
		{
			m_CommandLists[i].Construct(m_CommandPools[i].Get(), CommandListSpecification()
				.SetDebugName(std::format("CommandList for: {0}", m_CommandPools[i]->GetSpecification().DebugName))
			);
		}

		// Renderpass
		m_Renderpass.Construct(m_Device.Get(), RenderpassSpecification()
			.SetBindpoint(PipelineBindpoint::Graphics)

			.SetColourImageSpecification(m_Swapchain->GetImage(0).GetSpecification())
			.SetColourLoadOperation(LoadOperation::Clear)
			.SetColourStoreOperation(StoreOperation::Store)
			.SetColourStartState(ResourceState::Present)
			.SetColourRenderingState(ResourceState::RenderTarget)
			.SetColourEndState(ResourceState::Present)

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

		// ShaderCompiler & Shader
		ShaderCompiler compiler;
		std::vector<uint32_t> vertexSPIRV = compiler.CompileToSPIRV(ShaderStage::Vertex, std::string(g_VertexShader), "main", g_ShadingLanguage);
		std::vector<uint32_t> fragmentSPIRV = compiler.CompileToSPIRV(ShaderStage::Fragment, std::string(g_FragmentShader), "main", g_ShadingLanguage);

		Shader vertexShader = m_Device->CreateShader(ShaderSpecification()
			.SetShaderStage(ShaderStage::Vertex)
			.SetMainName("main")
			.SetSPIRV(vertexSPIRV)
			.SetPushConstantsInfo(0, 0, sizeof(Camera2DData))
			.SetDebugName("Vertex Shader")
		);
		Shader fragmentShader = m_Device->CreateShader(ShaderSpecification()
			.SetShaderStage(ShaderStage::Fragment)
			.SetMainName("main")
			.SetSPIRV(fragmentSPIRV)
			.SetDebugName("Fragment Shader")
		);

		// Input & Binding layout
		m_InputLayout.Construct(m_Device.Get(), std::initializer_list{
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
		});

		m_BindingLayoutSet0.Construct(m_Device.Get(), BindingLayoutSpecification()
			.SetRegisterSpace(0)

			// Vertex
			.AddItem(BindingLayoutItem()
				.SetSlot(0)
				.SetVisibility(ShaderStage::Vertex)
				.SetType(ResourceType::PushConstants)
				.SetSize(sizeof(Camera2DData))
				.SetDebugName("u_Camera2D")
			)

			.AddItem(BindingLayoutItem()
				.SetSlot(1)
				.SetVisibility(ShaderStage::Vertex)
				.SetType(ResourceType::DynamicUniformBuffer)
				.SetSize(3)
				.SetDebugName("u_Model")
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
						.SetSrcBlend(BlendFactor::SrcAlpha)
						.SetDstBlend(BlendFactor::OneMinusSrcAlpha)
						.SetBlendOperation(BlendOperation::Add)
						.SetSrcBlendAlpha(BlendFactor::One)
						.SetDstBlendAlpha(BlendFactor::OneMinusSrcAlpha)
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

		// Destroy shaders
		m_Device->DestroyShader(vertexShader);
		m_Device->DestroyShader(fragmentShader);

		// Init resources
		{
			CommandList initCommand = m_CommandPools[0]->AllocateList(CommandListSpecification()
				.SetDebugName("InitCommand")
			);
			initCommand.Open();

			// Buffers
			// StagingBuffer
			Buffer stagingBuffer = m_Device->CreateBuffer(BufferSpecification()
				.SetSize(sizeof(g_VertexData) + sizeof(g_IndexData) + (Nano::Memory::AlignOffset(sizeof(ModelData), BufferSpecification::DefaultUniformBufferAlignment) * 3))
				.SetCPUAccess(CpuAccessMode::Write)
			);
			m_Device->StartTracking(stagingBuffer, ResourceState::Unknown);

			void* bufferMemory;
			m_Device->MapBuffer(stagingBuffer, bufferMemory);

			m_VertexBuffer.Construct(m_Device.Get(), BufferSpecification()
				.SetSize(sizeof(g_VertexData))
				.SetIsVertexBuffer(true)
				.SetDebugName("Vertexbuffer")
			);
			m_Device->StartTracking(m_VertexBuffer.Get(), ResourceState::VertexBuffer);
			if (bufferMemory) std::memcpy(bufferMemory, static_cast<const void*>(g_VertexData.data()), sizeof(g_VertexData));
			initCommand.CopyBuffer(m_VertexBuffer.Get(), stagingBuffer, sizeof(g_VertexData));

			m_IndexBuffer.Construct(m_Device.Get(), BufferSpecification()
				.SetSize(sizeof(g_IndexData))
				.SetFormat(Format::R32UInt)
				.SetIsIndexBuffer(true)
				.SetDebugName("Indexbuffer")
			);
			m_Device->StartTracking(m_IndexBuffer.Get(), ResourceState::IndexBuffer);
			if (bufferMemory) std::memcpy(static_cast<uint8_t*>(bufferMemory) + sizeof(g_VertexData), g_IndexData.data(), sizeof(g_IndexData));
			initCommand.CopyBuffer(m_IndexBuffer.Get(), stagingBuffer, sizeof(g_IndexData), sizeof(g_VertexData));

			m_UniformBuffer.Construct(m_Device.Get(), BufferSpecification()
				.SetSize(0)
				.SetStride(sizeof(ModelData))
				.SetElementCount(3)
				.SetIsUniformBuffer(true)
				.SetIsDynamic(true)
				.SetDebugName("DynamicUniform")
			);
			m_Device->StartTracking(m_UniformBuffer.Get(), ResourceState::Unknown);

			Maths::Mat4<float> matrix = Maths::Mat4<float>(1.0f);
			m_Device->WriteBuffer(stagingBuffer, &matrix, sizeof(ModelData), 0, sizeof(g_VertexData) + sizeof(g_IndexData));

			matrix = Maths::Translate(matrix, { 1.0f, 0.0f, 0.0f });
			m_Device->WriteBuffer(stagingBuffer, &matrix, sizeof(ModelData), 0, sizeof(g_VertexData) + sizeof(g_IndexData) + Nano::Memory::AlignOffset(sizeof(ModelData), m_UniformBuffer->GetAlignment()));
			
			matrix = Maths::Translate(matrix, { -2.0f, -1.0f, 0.0f });
			m_Device->WriteBuffer(stagingBuffer, &matrix, sizeof(ModelData), 0, sizeof(g_VertexData) + sizeof(g_IndexData) + (Nano::Memory::AlignOffset(sizeof(ModelData), m_UniformBuffer->GetAlignment() * 2)));
				
			initCommand.CopyBuffer(m_UniformBuffer.Get(), stagingBuffer, (Nano::Memory::AlignOffset(sizeof(ModelData), m_UniformBuffer->GetAlignment()) * 3), sizeof(g_VertexData) + sizeof(g_IndexData));

			m_Camera2D.Construct(m_Window.Get());

			// Destroy
			m_Device->UnmapBuffer(stagingBuffer);

			initCommand.Close();
			initCommand.Submit(CommandListSubmitArgs());

			// Set items
			for (size_t i = 0; i < m_Set0s.size(); i++)
			{
				m_Set0s[i]->SetItem(1, m_UniformBuffer.Get(), BufferRange());
			}

			initCommand.WaitTillComplete();

			m_Device->DestroyBuffer(stagingBuffer);
		}
	}

	~DynamicUniformBuffer()
	{
		m_Device->DestroyBuffer(m_UniformBuffer.Get());
		m_Device->DestroyBuffer(m_IndexBuffer.Get());
		m_Device->DestroyBuffer(m_VertexBuffer.Get());

		m_Device->DestroyGraphicsPipeline(m_Pipeline.Get());

		m_Device->FreeBindingSetPool(m_BindingSetPool0.Get());

		m_Device->DestroyBindingLayout(m_BindingLayoutSet0.Get());
		m_Device->DestroyInputLayout(m_InputLayout.Get());

		m_Device->DestroyRenderpass(m_Renderpass.Get());

		for (size_t i = 0; i < m_CommandPools.size(); i++)
		{
			m_CommandPools[i]->FreeList(m_CommandLists[i].Get());
			m_Swapchain->FreePool(m_CommandPools[i].Get());
		}
	}

	// Methods
	void Run()
	{
		while (m_Window->IsOpen())
		{
			m_Window->PollEvents();
			FreeQueue();

			// Update
			{
				double deltaTime = GetDeltaTime();
				m_Camera2D->OnUpdate(static_cast<float>(deltaTime));
			}

			// Rendering
			{
				m_Swapchain->AcquireNextImage();
				{
					m_CommandPools[m_Swapchain->GetCurrentFrame()]->Reset();
					auto& list = m_CommandLists[m_Swapchain->GetCurrentFrame()];

					list->Open();

					list->StartRenderpass(RenderpassStartArgs()
						.SetRenderpass(m_Renderpass.Get())

						.SetViewport(Viewport(static_cast<float>(m_Window->GetSize().x), static_cast<float>(m_Window->GetSize().y)))
						.SetScissor(ScissorRect(Viewport(static_cast<float>(m_Window->GetSize().x), static_cast<float>(m_Window->GetSize().y))))

						.SetColourClear({ 0.0f, 0.0f, 0.0f, 1.0f })
					);

					list->BindPipeline(m_Pipeline.Get());

					list->BindVertexBuffer(m_VertexBuffer.Get());
					list->BindIndexBuffer(m_IndexBuffer.Get());

					std::array<uint32_t, 1> offset = { 0 };
					list->BindBindingSet(m_Set0s[m_Swapchain->GetCurrentFrame()], offset);

					list->PushConstants(&m_Camera2D->GetCamera2D(), sizeof(Camera2DData));

					list->DrawIndexed(DrawArguments()
						.SetVertexCount((sizeof(g_IndexData) / sizeof(g_IndexData[0])))
						.SetInstanceCount(1)
					);

					offset[0] = static_cast<uint32_t>(Nano::Memory::AlignOffset(offset[0] + sizeof(ModelData), m_UniformBuffer->GetAlignment()));
					list->BindBindingSet(m_Set0s[m_Swapchain->GetCurrentFrame()], offset);
					list->DrawIndexed(DrawArguments()
						.SetVertexCount((sizeof(g_IndexData) / sizeof(g_IndexData[0])))
						.SetInstanceCount(1)
					);

					offset[0] = static_cast<uint32_t>(Nano::Memory::AlignOffset(offset[0] + sizeof(ModelData), m_UniformBuffer->GetAlignment()));
					list->BindBindingSet(m_Set0s[m_Swapchain->GetCurrentFrame()], offset);
					list->DrawIndexed(DrawArguments()
						.SetVertexCount((sizeof(g_IndexData) / sizeof(g_IndexData[0])))
						.SetInstanceCount(1)
					);

					list->EndRenderpass(RenderpassEndArgs()
						.SetRenderpass(m_Renderpass.Get())
					);

					list->Close();
					list->Submit(CommandListSubmitArgs()
						.SetWaitForSwapchainImage(true)
						.SetOnFinishMakeSwapchainPresentable(true)
					);
				}
				m_Swapchain->Present();
			}
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

		m_Camera2D->OnEvent(e);
	}

	void OnDeviceMessage(DeviceMessageType msgType, const std::string& message)
	{
		switch (msgType)
		{
		case DeviceMessageType::Warn:
			OB_LOG_WARN("Device Warning: {0}", message);
			break;
		case DeviceMessageType::Error:
			OB_LOG_ERROR("Device Error: {0}", message);
			break;

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

private:
	std::array<Nano::Memory::DeferredConstruct<CommandListPool>, Information::FramesInFlight> m_CommandPools = { };
	std::array<Nano::Memory::DeferredConstruct<CommandList>, Information::FramesInFlight> m_CommandLists = { };

	Nano::Memory::DeferredConstruct<Renderpass> m_Renderpass = {};

	Nano::Memory::DeferredConstruct<InputLayout> m_InputLayout = {};
	Nano::Memory::DeferredConstruct<BindingLayout> m_BindingLayoutSet0 = {};

	Nano::Memory::DeferredConstruct<BindingSetPool> m_BindingSetPool0 = {};
	std::array<Nano::Memory::DeferredConstruct<BindingSet>, Information::FramesInFlight> m_Set0s = {};
	Nano::Memory::DeferredConstruct<GraphicsPipeline> m_Pipeline = {};

	Nano::Memory::DeferredConstruct<Buffer> m_VertexBuffer = {};
	Nano::Memory::DeferredConstruct<Buffer> m_IndexBuffer = {};
	Nano::Memory::DeferredConstruct<Buffer> m_UniformBuffer = {};

	Nano::Memory::DeferredConstruct<Camera2D> m_Camera2D = {};

	std::queue<DeviceDestroyFn> m_DestroyQueue = {};
};

int Main(int argc, char* argv[])
{
	(void)argc; (void)argv;

	DynamicUniformBuffer app;
	app.Run();
	return 0;
}