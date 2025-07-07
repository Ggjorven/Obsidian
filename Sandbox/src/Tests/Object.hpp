#include "Tests/TestBase.hpp"
#include "Common/Camera3D.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include <tinyobj/tinyobjloader.h>

////////////////////////////////////////////////////////////////////////////////////
// Shaders
////////////////////////////////////////////////////////////////////////////////////
#if 1
inline constexpr ShadingLanguage g_ShadingLanguage = ShadingLanguage::GLSL;

inline constexpr std::string_view g_VertexShader = R"(
#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Colour;
layout(location = 2) in vec2 a_TexCoord;

layout(location = 0) out vec3 v_Position;
layout(location = 1) out vec4 v_Colour;
layout(location = 2) out vec2 v_TexCoord;

layout(std140, set = 0, binding = 0) uniform Camera3DSettings
{
    mat4 View;
    mat4 Projection;
} u_Camera3D;

void main()
{
    v_Position = a_Position;
    v_Colour = a_Colour;
    v_TexCoord = a_TexCoord;

    gl_Position = u_Camera3D.Projection * u_Camera3D.View * vec4(a_Position, 1.0);
}
)";

inline constexpr std::string_view g_FragmentShader = R"(
#version 460 core

layout(location = 0) out vec4 o_Colour;

layout(location = 0) in vec3 v_Position;
layout(location = 1) in vec4 v_Colour;
layout(location = 2) in vec2 v_TexCoord;

layout (set = 0, binding = 1) uniform texture2D u_Texture;
layout (set = 0, binding = 2) uniform sampler u_Sampler;

void main()
{
	// Combine texture and sampler
    o_Colour = v_Colour * texture(sampler2D(u_Texture, u_Sampler), v_TexCoord);
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

cbuffer u_Camera3D : register(b0, space0)
{
    float4x4 View;
    float4x4 Proj;
};

VSOutput main(VSInput input) 
{
    VSOutput output;
    output.v_TexCoord = input.a_TexCoord;

	float4 worldPos = float4(input.a_Position, 1.0);
    output.gl_Position = mul(Proj, mul(View, worldPos));
    return output;
}
)";

inline constexpr std::string_view g_FragmentShader = R"(
struct PSInput 
{
    float2 v_TexCoord : TEXCOORD0;
};

Texture2D u_Texture : register(t1, space0);
SamplerState u_Sampler : register(s2, space0);

float4 main(PSInput input) : SV_TARGET 
{
    //return float4(input.v_TexCoord.x, input.v_TexCoord.y, 0.0, 1.0);
    return u_Texture.Sample(u_Sampler, input.v_TexCoord);
}
)";

#endif

////////////////////////////////////////////////////////////////////////////////////
// Vertex data
////////////////////////////////////////////////////////////////////////////////////
struct Vertex
{
public:
	Maths::Vec3<float> Position = { 0.0f, 0.0f, 0.0f };
	Maths::Vec4<float> Colour = { 1.0f, 1.0f, 1.0f, 1.0f };
	Maths::Vec2<float> TexCoord;

public:
	// Operators
	inline constexpr bool operator == (const Vertex& other) const { return ((Position == other.Position) && (Colour == other.Colour) && (TexCoord == other.TexCoord)); }
	inline constexpr bool operator != (const Vertex& other) const { return !(*this == other); }
};

namespace std 
{
	template<> 
	struct hash<Vertex> 
	{
		size_t operator () (Vertex const& vertex) const 
		{
			return ((hash<Maths::Vec3<float>>()(vertex.Position) ^ (hash<Maths::Vec4<float>>()(vertex.Colour) << 1)) >> 1) ^ (hash<Maths::Vec2<float>>()(vertex.TexCoord) << 1);
		}
	};
}

////////////////////////////////////////////////////////////////////////////////////
// Test
////////////////////////////////////////////////////////////////////////////////////
class Object : public TestBase
{
public:
	// Constructor & Destructor
	Object()
		: TestBase(1280, 720, "Object", [this](Event e) { OnEvent(e); }, [this](DeviceMessageType type, const std::string& msg) { OnDeviceMessage(type, msg); })
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

		Shader vertexShader = m_Device->CreateShader({ ShaderStage::Vertex, "main", vertexSPIRV, "Vertex Shader" });
		Shader fragmentShader = m_Device->CreateShader({ ShaderStage::Fragment, "main", fragmentSPIRV, "Fragment Shader" });

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
				.SetFormat(Format::RGBA32Float)
				.SetSize(VertexAttributeSpecification::AutoSize)
				.SetOffset(VertexAttributeSpecification::AutoOffset)
				.SetDebugName("a_Colour"),
			VertexAttributeSpecification()
				.SetBufferIndex(0)
				.SetLocation(2)
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
				.SetDebugName("u_Camera3D")
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

		// Loading
		std::unordered_map<Vertex, uint32_t> uniqueVertices;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		{
			std::string inputfile = "resources/objects/viking_room.obj";
			tinyobj::ObjReader reader;

			bool success = reader.ParseFromFile(inputfile);

			if (!success && !reader.Error().empty())
				NG_ASSERT(false, "{0}", reader.Error());

			auto& attrib = reader.GetAttrib();
			auto& shapes = reader.GetShapes();

			for (const auto& shape : shapes) 
			{
				vertices.reserve(attrib.vertices.size());
				uniqueVertices.reserve(vertices.capacity());
				indices.reserve(shape.mesh.indices.size());

				for (const auto& index : shape.mesh.indices) 
				{
					Vertex vertex = {};

					vertex.Position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					// Note: We don't need to touch the colour

					vertex.TexCoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						attrib.texcoords[2 * index.texcoord_index + 1]
					};

					vertices.push_back(vertex);

					// Cache the vertex
					if (!uniqueVertices.contains(vertex)) 
					{
						uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
						vertices.push_back(vertex);
					}

					indices.push_back(uniqueVertices[vertex]);
				}
			}
		}

		// Init resources
		{
			CommandList initCommand = m_CommandPools[0]->AllocateList(CommandListSpecification()
				.SetDebugName("InitCommand")
			);
			initCommand.Open();

			// Buffers
			// StagingBuffer
			Buffer stagingBuffer = m_Device->CreateBuffer(BufferSpecification()
				.SetSize((vertices.size() * sizeof(Vertex)) + (indices.size() * sizeof(uint32_t)))
				.SetCPUAccess(CpuAccessMode::Write)
			);
			m_Device->StartTracking(stagingBuffer, ResourceState::Unknown);

			void* bufferMemory;
			m_Device->MapBuffer(stagingBuffer, bufferMemory);

			m_VertexBuffer.Construct(m_Device.Get(), BufferSpecification()
				.SetSize((vertices.size() * sizeof(Vertex)))
				.SetIsVertexBuffer(true)
				.SetDebugName("Vertexbuffer")
			);
			m_Device->StartTracking(m_VertexBuffer.Get(), ResourceState::VertexBuffer);
			if (bufferMemory) std::memcpy(bufferMemory, static_cast<const void*>(vertices.data()), (vertices.size() * sizeof(Vertex)));
			initCommand.CopyBuffer(m_VertexBuffer.Get(), stagingBuffer, (vertices.size() * sizeof(Vertex)));

			m_IndexBuffer.Construct(m_Device.Get(), BufferSpecification()
				.SetSize((indices.size() * sizeof(uint32_t)))
				.SetFormat(Format::R32UInt)
				.SetIsIndexBuffer(true)
				.SetDebugName("Indexbuffer")
			);
			m_Device->StartTracking(m_IndexBuffer.Get(), ResourceState::IndexBuffer);
			if (bufferMemory) std::memcpy(static_cast<uint8_t*>(bufferMemory) + (vertices.size() * sizeof(Vertex)), indices.data(), (indices.size() * sizeof(uint32_t)));
			initCommand.CopyBuffer(m_IndexBuffer.Get(), stagingBuffer, (indices.size() * sizeof(uint32_t)), (vertices.size() * sizeof(Vertex)));

			// Image & Sampler
			// StagingImage
			StagingImage stagingImage = m_Device->CreateStagingImage(ImageSpecification()
				.SetImageFormat(Format::RGBA8Unorm)
				.SetWidthAndHeight(1, 1)
				.SetImageDimension(ImageDimension::Image2D),
				CpuAccessMode::Write
			);
			m_Device->StartTracking(stagingImage, ResourceState::Unknown);

			m_Image.Construct(m_Device.Get(), ImageSpecification()
				.SetImageFormat(Format::RGBA8Unorm)
				.SetImageDimension(ImageDimension::Image2D)
				.SetPermanentState(ResourceState::ShaderResource)
				.SetIsShaderResource(true)
				.SetWidthAndHeight(1, 1)
				.SetMipLevels(1)
				.SetDebugName("Temp image")
			);
			m_Device->StartTracking(m_Image.Get(), ImageSubresourceSpecification(0, 1, 0, 1), ResourceState::Unknown);

			// TODO: Image
			uint32_t colour = 0xFFFFFFFF;
			m_Device->WriteImage(stagingImage, ImageSliceSpecification(), &colour, sizeof(uint32_t));
			
			initCommand.CopyImage(m_Image.Get(), ImageSliceSpecification(), stagingImage, ImageSliceSpecification());

			m_Sampler.Construct(m_Device.Get(), SamplerSpecification().SetDebugName(std::format("Sampler for: {0}", m_Image->GetSpecification().DebugName)));

			// Uniformbuffer
			m_UniformBuffer.Construct(m_Device.Get(), BufferSpecification()
				.SetSize(sizeof(Camera3DData))
				.SetIsUniformBuffer(true)
				.SetCPUAccess(CpuAccessMode::Write)
				.SetDebugName("Camera3D Uniform")
			);
			m_Device->StartTracking(m_UniformBuffer.Get(), ResourceState::Unknown);

			m_Device->MapBuffer(m_UniformBuffer.Get(), m_UniformMemory);

			m_Camera3D.Construct(m_Window.Get());

			if (m_UniformMemory)
				std::memcpy(static_cast<uint8_t*>(m_UniformMemory), &m_Camera3D->GetCamera3D(), sizeof(Camera3DData));

			// Destroy
			m_Device->UnmapBuffer(stagingBuffer);

			initCommand.Close();
			initCommand.Submit(CommandListSubmitArgs());

			// Upload to bindinsets
			for (auto& set : m_Set0s)
			{
				set->SetItem(0, m_UniformBuffer.Get(), BufferRange());
				set->SetItem(1, m_Image.Get(), ImageSubresourceSpecification());
				set->SetItem(2, m_Sampler.Get());
			}

			initCommand.WaitTillComplete();

			m_Device->DestroyBuffer(stagingBuffer);
			m_Device->DestroyStagingImage(stagingImage);
		}
	}

	~Object()
	{
		m_Device->UnmapBuffer(m_UniformBuffer.Get());

		m_Device->DestroyBuffer(m_UniformBuffer.Get());

		m_Device->DestroySampler(m_Sampler.Get());
		m_Device->DestroyImage(m_Image.Get());

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
				m_Camera3D->OnUpdate(static_cast<float>(deltaTime));

				if (m_UniformMemory)
					std::memcpy(static_cast<uint8_t*>(m_UniformMemory), &m_Camera3D->GetCamera3D(), sizeof(Camera3DData));
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

						.SetColourClear({ 1.0f, 0.0f, 0.0f, 1.0f })
					);

					list->BindPipeline(m_Pipeline.Get());

					list->BindVertexBuffer(m_VertexBuffer.Get());
					list->BindIndexBuffer(m_IndexBuffer.Get());

					list->BindBindingSet(m_Set0s[m_Swapchain->GetCurrentFrame()]);

					list->DrawIndexed(DrawArguments()
						.SetVertexCount(static_cast<uint32_t>(m_IndexBuffer->GetSpecification().Size / sizeof(uint32_t)))
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

		m_Camera3D->OnEvent(e);
	}

	void OnDeviceMessage(DeviceMessageType msgType, const std::string& message)
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
		m_Camera3D->OnUpdate(deltaTime);

		if (m_UniformMemory)
			std::memcpy(static_cast<uint8_t*>(m_UniformMemory), &m_Camera3D->GetCamera3D(), sizeof(Camera3DData));
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

	Nano::Memory::DeferredConstruct<Image> m_Image = {};
	Nano::Memory::DeferredConstruct<Sampler> m_Sampler = {};

	void* m_UniformMemory;
	Nano::Memory::DeferredConstruct<Buffer> m_UniformBuffer = {};
	Nano::Memory::DeferredConstruct<Camera3D> m_Camera3D = {};

	std::queue<DeviceDestroyFn> m_DestroyQueue = {};
};

int Main(int argc, char* argv[])
{
	(void)argc; (void)argv;

	Object app;
	app.Run();
	return 0;
}