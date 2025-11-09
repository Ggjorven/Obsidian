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

layout(std140, set = 0, binding = 0) uniform Camera2DSettings
{
    mat4 View;
    mat4 Projection;
	mat4 Model;
} u_Camera2D;

void main()
{
    v_Position = a_Position;
    v_TexCoord = a_TexCoord;

    gl_Position = u_Camera2D.Projection * u_Camera2D.View * u_Camera2D.Model * vec4(a_Position, 1.0);
    //gl_Position = vec4(a_Position, 1.0);
}
)";

inline constexpr std::string_view g_FragmentShader = R"(
#version 460 core

layout(location = 0) out vec4 o_Colour;

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec2 v_TexCoord;

void main()
{
	// Combine texture and sampler
	o_Colour = vec4(v_TexCoord.x, v_TexCoord.y, 0.0, 1.0);
}
)";
#else
inline constexpr ShadingLanguage g_ShadingLanguage = ShadingLanguage::HLSL;

inline constexpr std::string_view g_VertexShader = R"(
struct VSInput 
{
    float3 a_Position : POSITION;
    float2 a_TexCoord : TEXCOORD0;
};

struct VSOutput 
{
    float4 gl_Position : SV_POSITION;
    float2 v_TexCoord  : TEXCOORD0;
};

cbuffer u_Camera2D : register(b0, space0)
{
    float4x4 View;
    float4x4 Proj;
    float4x4 Model;
};

VSOutput main(VSInput input) 
{
    VSOutput output;
    output.v_TexCoord = input.a_TexCoord;

	float4 worldPos = float4(input.a_Position, 1.0);
    output.gl_Position = mul(Proj, mul(View, mul(Model, worldPos)));
    return output;
    
	//output.gl_Position = float4(input.a_Position, 1.0);
    //return output;
}
)";

inline constexpr std::string_view g_FragmentShader = R"(
struct PSInput 
{
    float2 v_TexCoord : TEXCOORD0;
};

float4 main(PSInput input) : SV_TARGET 
{
    return float4(input.v_TexCoord.x, input.v_TexCoord.y, 0.0, 1.0);
    //return u_Texture.Sample(u_Sampler, input.v_TexCoord);
}
)";

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
// Test
////////////////////////////////////////////////////////////////////////////////////
class Renderpasses : public TestBase
{
public:
	// Constructor & Destructor
	Renderpasses()
		: TestBase(1280, 720, "Renderpasses", [this](Event e) { OnEvent(e); }, [this](DeviceMessageType type, const std::string& msg) { OnDeviceMessage(type, msg); })
	{
		// Commandpools & Commandlists
		for (auto& pool : m_CommandPools)
			pool.Construct(m_Swapchain.Get(), CommandListPoolSpecification()
				.SetQueue(CommandQueue::Graphics)
				.SetDebugName("CommandPool")
			);

		for (size_t i = 0; i < m_Pass0Lists.size(); i++)
		{
			m_Pass0Lists[i].Construct(m_CommandPools[i].Get(), CommandListSpecification()
				.SetDebugName(std::format("CommandList for: {0}", m_CommandPools[i]->GetSpecification().DebugName))
			);
		}

		for (size_t i = 0; i < m_Pass1Lists.size(); i++)
		{
			m_Pass1Lists[i].Construct(m_CommandPools[i].Get(), CommandListSpecification()
				.SetDebugName(std::format("CommandList for: {0}", m_CommandPools[i]->GetSpecification().DebugName))
			);
		}

		// Renderpasses
		m_Pass0.Construct(m_Device.Get(), RenderpassSpecification()
			.SetBindpoint(PipelineBindpoint::Graphics)

			.SetColourImageSpecification(m_Swapchain->GetImage(0).GetSpecification())
			.SetColourLoadOperation(LoadOperation::Clear)
			.SetColourStoreOperation(StoreOperation::Store)
			.SetColourStartState(ResourceState::Present)
			.SetColourRenderingState(ResourceState::RenderTarget)
			.SetColourEndState(ResourceState::RenderTarget)

			.SetDebugName("Pass0")
		);

		for (size_t i = 0; i < m_Swapchain->GetImageCount(); i++)
		{
			std::string debugName = std::format("Framebuffer({0}) for: {1}", i, m_Pass0->GetSpecification().DebugName);
			(void)m_Pass0->CreateFramebuffer(FramebufferSpecification()
				.SetColourAttachment(FramebufferAttachment()
					.SetImage(m_Swapchain->GetImage(static_cast<uint8_t>(i)))
				)
				.SetDebugName(debugName)
			);
		}

		m_Pass1.Construct(m_Device.Get(), RenderpassSpecification()
			.SetBindpoint(PipelineBindpoint::Graphics)

			.SetColourImageSpecification(m_Swapchain->GetImage(0).GetSpecification())
			.SetColourLoadOperation(LoadOperation::Load)
			.SetColourStoreOperation(StoreOperation::Store)
			.SetColourStartState(ResourceState::RenderTarget)
			.SetColourRenderingState(ResourceState::RenderTarget)
			.SetColourEndState(ResourceState::Present)

			.SetDebugName("Pass1")
		);

		for (size_t i = 0; i < m_Swapchain->GetImageCount(); i++)
		{
			std::string debugName = std::format("Framebuffer({0}) for: {1}", i, m_Pass1->GetSpecification().DebugName);
			(void)m_Pass1->CreateFramebuffer(FramebufferSpecification()
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
				.SetType(ResourceType::UniformBuffer)
				.SetDebugName("u_Camera2D")
			)

			.SetDebugName("Layout for: Set0")
		);

		// BindingPool & Sets
		m_BindingSetPool0.Construct(m_Device.Get(), BindingSetPoolSpecification()
			.SetLayout(m_BindingLayoutSet0.Get())
			.SetSetAmount(Information::FramesInFlight * 2)
			.SetDebugName("BindingSetPool0")
		);
		for (size_t i = 0; i < m_Set0sforPass0.size(); i++)
		{
			m_Set0sforPass0[i].Construct(m_BindingSetPool0.Get(), BindingSetSpecification());
		}
		for (size_t i = 0; i < m_Set0sforPass1.size(); i++)
		{
			m_Set0sforPass1[i].Construct(m_BindingSetPool0.Get(), BindingSetSpecification());
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

			.SetRenderpass(m_Pass0.Get())
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
				.SetSize(sizeof(g_VertexData) + sizeof(g_IndexData))
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

			// Uniformbuffer
			m_UniformBufferForPass0.Construct(m_Device.Get(), BufferSpecification()
				.SetSize(sizeof(Camera2DData) + sizeof(Maths::Mat4<float>))
				.SetIsUniformBuffer(true)
				.SetCPUAccess(CpuAccessMode::Write)
				.SetDebugName("Camera2D Uniform")
			);
			m_Device->StartTracking(m_UniformBufferForPass0.Get(), ResourceState::Unknown);

			m_UniformBufferForPass1.Construct(m_Device.Get(), BufferSpecification()
				.SetSize(sizeof(Camera2DData) + sizeof(Maths::Mat4<float>))
				.SetIsUniformBuffer(true)
				.SetCPUAccess(CpuAccessMode::Write)
				.SetDebugName("Camera2D Uniform")
			);
			m_Device->StartTracking(m_UniformBufferForPass1.Get(), ResourceState::Unknown);

			m_Device->MapBuffer(m_UniformBufferForPass0.Get(), m_UniformMemoryForPass0);
			m_Device->MapBuffer(m_UniformBufferForPass1.Get(), m_UniformMemoryForPass1);

			m_Camera2D.Construct(m_Window.Get());

			// Destroy
			m_Device->UnmapBuffer(stagingBuffer);

			initCommand.Close();
			initCommand.Submit(CommandListSubmitArgs());

			// Upload to bindinsets
			for (auto& set : m_Set0sforPass0)
			{
				set->SetItem(0, m_UniformBufferForPass0.Get(), BufferRange());
			}
			for (auto& set : m_Set0sforPass1)
			{
				set->SetItem(0, m_UniformBufferForPass1.Get(), BufferRange());
			}

			initCommand.WaitTillComplete();

			m_Device->DestroyBuffer(stagingBuffer);
		}
	}

	~Renderpasses()
	{
		m_Device->UnmapBuffer(m_UniformBufferForPass0.Get());
		m_Device->UnmapBuffer(m_UniformBufferForPass1.Get());

		m_Device->DestroyBuffer(m_UniformBufferForPass0.Get());
		m_Device->DestroyBuffer(m_UniformBufferForPass1.Get());

		m_Device->DestroyBuffer(m_IndexBuffer.Get());
		m_Device->DestroyBuffer(m_VertexBuffer.Get());

		m_Device->DestroyGraphicsPipeline(m_Pipeline.Get());

		m_Device->FreeBindingSetPool(m_BindingSetPool0.Get());

		m_Device->DestroyBindingLayout(m_BindingLayoutSet0.Get());
		m_Device->DestroyInputLayout(m_InputLayout.Get());

		m_Device->DestroyRenderpass(m_Pass0.Get());
		m_Device->DestroyRenderpass(m_Pass1.Get());

		for (size_t i = 0; i < m_CommandPools.size(); i++)
		{
			m_CommandPools[i]->FreeList(m_Pass0Lists[i].Get());
			m_CommandPools[i]->FreeList(m_Pass1Lists[i].Get());
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

				if (m_UniformMemoryForPass0)
					std::memcpy(static_cast<uint8_t*>(m_UniformMemoryForPass0), &m_Camera2D->GetCamera2D(), sizeof(Camera2DData));
				if (m_UniformMemoryForPass1)
					std::memcpy(static_cast<uint8_t*>(m_UniformMemoryForPass1), &m_Camera2D->GetCamera2D(), sizeof(Camera2DData));
			}

			// Rendering
			{
				m_Swapchain->AcquireNextImage();
				{
					m_CommandPools[m_Swapchain->GetCurrentFrame()]->Reset();
					auto& list0 = m_Pass0Lists[m_Swapchain->GetCurrentFrame()];
					auto& list1 = m_Pass1Lists[m_Swapchain->GetCurrentFrame()];

					// First pass
					{
						list0->Open();

						list0->StartRenderpass(RenderpassStartArgs()
							.SetRenderpass(m_Pass0.Get())

							.SetViewport(Viewport(static_cast<float>(m_Window->GetSize().x), static_cast<float>(m_Window->GetSize().y)))
							.SetScissor(ScissorRect(Viewport(static_cast<float>(m_Window->GetSize().x), static_cast<float>(m_Window->GetSize().y))))

							.SetColourClear({ 0.0f, 0.0f, 0.0f, 1.0f })
						);

						list0->BindPipeline(m_Pipeline.Get());

						list0->BindVertexBuffer(m_VertexBuffer.Get());
						list0->BindIndexBuffer(m_IndexBuffer.Get());
						
						if (m_UniformMemoryForPass0)
						{
							Maths::Mat4<float> pos = Maths::Mat4<float>(1.0f);
							std::memcpy(static_cast<uint8_t*>(m_UniformMemoryForPass0) + sizeof(Camera2DData), &pos[0][0], sizeof(Maths::Mat4<float>));
						}

						list0->BindBindingSet(m_Set0sforPass0[m_Swapchain->GetCurrentFrame()]);

						list0->DrawIndexed(DrawArguments()
							.SetVertexCount((sizeof(g_IndexData) / sizeof(g_IndexData[0])))
							.SetInstanceCount(1)
						);

						list0->EndRenderpass(RenderpassEndArgs()
							.SetRenderpass(m_Pass0.Get())
						);

						list0->Close();
						list0->Submit(CommandListSubmitArgs()
							.SetWaitForSwapchainImage(true)
							.SetOnFinishMakeSwapchainPresentable(false)
						);
					}

					// Second pass
					{
						list1->Open();

						list1->StartRenderpass(RenderpassStartArgs()
							.SetRenderpass(m_Pass1.Get())

							.SetViewport(Viewport(static_cast<float>(m_Window->GetSize().x), static_cast<float>(m_Window->GetSize().y)))
							.SetScissor(ScissorRect(Viewport(static_cast<float>(m_Window->GetSize().x), static_cast<float>(m_Window->GetSize().y))))
						);

						list1->BindPipeline(m_Pipeline.Get());

						list1->BindVertexBuffer(m_VertexBuffer.Get());
						list1->BindIndexBuffer(m_IndexBuffer.Get());

						if (m_UniformMemoryForPass1)
						{
							Maths::Mat4<float> pos = Maths::Mat4<float>(1.0f);

							pos = Maths::Translate(pos, { 0.5, 0.0f, 0.0f });

							std::memcpy(static_cast<uint8_t*>(m_UniformMemoryForPass1) + sizeof(Camera2DData), &pos[0][0], sizeof(Maths::Mat4<float>));
						}

						list1->BindBindingSet(m_Set0sforPass1[m_Swapchain->GetCurrentFrame()]);

						list1->DrawIndexed(DrawArguments()
							.SetVertexCount((sizeof(g_IndexData) / sizeof(g_IndexData[0])))
							.SetInstanceCount(1)
						);

						list1->EndRenderpass(RenderpassEndArgs()
							.SetRenderpass(m_Pass1.Get())
						);

						list1->Close();
						list1->Submit(CommandListSubmitArgs()
							.SetWaitOnLists({ &list0.Get() })
							.SetWaitForSwapchainImage(false)
							.SetOnFinishMakeSwapchainPresentable(true)
						);
					}
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
			m_Pass0->ResizeFramebuffers();
			m_Pass1->ResizeFramebuffers();
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
	std::array<Nano::Memory::DeferredConstruct<CommandList>, Information::FramesInFlight> m_Pass0Lists = { };
	std::array<Nano::Memory::DeferredConstruct<CommandList>, Information::FramesInFlight> m_Pass1Lists = { };

	Nano::Memory::DeferredConstruct<Renderpass> m_Pass0 = {};
	Nano::Memory::DeferredConstruct<Renderpass> m_Pass1 = {};

	Nano::Memory::DeferredConstruct<InputLayout> m_InputLayout = {};
	Nano::Memory::DeferredConstruct<BindingLayout> m_BindingLayoutSet0 = {};

	Nano::Memory::DeferredConstruct<BindingSetPool> m_BindingSetPool0 = {};
	std::array<Nano::Memory::DeferredConstruct<BindingSet>, Information::FramesInFlight> m_Set0sforPass0 = {};
	std::array<Nano::Memory::DeferredConstruct<BindingSet>, Information::FramesInFlight> m_Set0sforPass1 = {};
	Nano::Memory::DeferredConstruct<GraphicsPipeline> m_Pipeline = {};

	Nano::Memory::DeferredConstruct<Buffer> m_VertexBuffer = {};
	Nano::Memory::DeferredConstruct<Buffer> m_IndexBuffer = {};

	void* m_UniformMemoryForPass0;
	void* m_UniformMemoryForPass1;
	Nano::Memory::DeferredConstruct<Buffer> m_UniformBufferForPass0 = {};
	Nano::Memory::DeferredConstruct<Buffer> m_UniformBufferForPass1 = {};
	Nano::Memory::DeferredConstruct<Camera2D> m_Camera2D = {};

	std::queue<DeviceDestroyFn> m_DestroyQueue = {};
};

int Main(int argc, char* argv[])
{
	(void)argc; (void)argv;

	Renderpasses app;
	app.Run();
	return 0;
}
