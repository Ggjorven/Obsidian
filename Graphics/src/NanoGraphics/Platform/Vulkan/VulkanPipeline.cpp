#include "ngpch.h"
#include "VulkanPipeline.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Renderer/Device.hpp"
#include "NanoGraphics/Renderer/Bindings.hpp"
#include "NanoGraphics/Renderer/Pipeline.hpp"
#include "NanoGraphics/Renderer/Renderpass.hpp"

namespace Nano::Graphics::Internal
{

	namespace
	{

		////////////////////////////////////////////////////////////////////////////////////
		// Helper function
		////////////////////////////////////////////////////////////////////////////////////
		void CreatePipelineLayout(VkPipelineLayout& layout, const Nano::Memory::StaticVector<BindingLayout*, GraphicsPipelineSpecification::MaxBindings>& layouts, VkDevice device, VkShaderStageFlags& outPushConstantsStage)
		{
			// Descriptor layouts
			std::vector<VkDescriptorSetLayout> descriptorLayouts;
			descriptorLayouts.reserve(GraphicsPipelineSpecification::MaxBindings);
			std::optional<VkPushConstantRange> range;

			for (auto descriptorLayout : layouts)
			{
				VulkanBindingLayout& vulkanLayout = *api_cast<VulkanBindingLayout*>(descriptorLayout);
				descriptorLayouts.push_back(vulkanLayout.GetVkDescriptorSetLayout());

				// Only iterate over all the items when we don't already have a range
				if (!range.has_value())
				{
					for (const auto& item : vulkanLayout.GetBindingItems())
					{
						if (item.Type != ResourceType::PushConstants)
							continue;

						NG_ASSERT((item.Size <= 128), "[VkxxxPipeline] Push constants size has to be less than 128 bytes.");
						NG_ASSERT((item.Size % 4 == 0), "[VkxxxPipeline] Push constants size must be aligned to 4 bytes.");

						// Note: We currently only support 1 push constant range, so
						// the offset is 0
						VkPushConstantRange& vkRange = range.emplace();
						vkRange.stageFlags = ShaderStageToVkShaderStageFlags(item.Visibility);
						vkRange.offset = 0;
						vkRange.size = item.Size;

						outPushConstantsStage = vkRange.stageFlags;
					}
				}
			}

			VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(range.has_value());
			pipelineLayoutInfo.pPushConstantRanges = (range.has_value() ? &range.value() : nullptr);
			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorLayouts.size());
			pipelineLayoutInfo.pSetLayouts = descriptorLayouts.data();

			VK_VERIFY(vkCreatePipelineLayout(device, &pipelineLayoutInfo, VulkanAllocator::GetCallbacks(), &layout));
		}

	}

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
    VulkanGraphicsPipeline::VulkanGraphicsPipeline(const Device& device, const GraphicsPipelineSpecification& specs)
        : m_Specification(specs)
    {
		NG_ASSERT(specs.Input, "[VkGraphicsPipeline] No proper inputlayout specified.");
		NG_ASSERT(specs.Pass, "[VkGraphicsPipeline] No proper renderpass specified.");

		const VulkanDevice& vulkanDevice = *api_cast<const VulkanDevice*>(&device);

		auto& blendState = m_Specification.RenderingState.Blend;
		auto& depthStencilState = m_Specification.RenderingState.DepthStencil;
		auto& rasterState = m_Specification.RenderingState.Raster;

		VulkanRenderpass& vulkanRenderpass = *api_cast<VulkanRenderpass*>(m_Specification.Pass);
		NG_ASSERT(!vulkanRenderpass.GetFramebuffers().empty(), "[VkGraphicsPipeline] Renderpass passed in with no created framebuffers.");
		VulkanFramebuffer& vulkanFramebuffer = *api_cast<VulkanFramebuffer*>(&vulkanRenderpass.GetFramebuffer(0));

		Nano::Memory::StaticVector<VkPipelineShaderStageCreateInfo, 5> shaderStages = { }; // Note: Update when more shaders are added

		const auto generateShaderCreateInfo = [&](VkShaderStageFlags shaderStage, VkShaderModule mod, const char* main)
		{
			VkPipelineShaderStageCreateInfo& vertShaderStageInfo = shaderStages.emplace_back();
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = static_cast<VkShaderStageFlagBits>(shaderStage);
			vertShaderStageInfo.module = mod;
			vertShaderStageInfo.pName = main;
		};

		if (m_Specification.VertexShader) generateShaderCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, api_cast<VulkanShader*>(m_Specification.VertexShader)->GetVkShaderModule(), m_Specification.VertexShader->GetSpecification().MainName.data());
		if (m_Specification.TesselationControlShader) generateShaderCreateInfo(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, api_cast<VulkanShader*>(m_Specification.TesselationControlShader)->GetVkShaderModule(), m_Specification.TesselationControlShader->GetSpecification().MainName.data());
		if (m_Specification.TesselationEvaluationShader) generateShaderCreateInfo(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, api_cast<VulkanShader*>(m_Specification.TesselationEvaluationShader)->GetVkShaderModule(), m_Specification.TesselationEvaluationShader->GetSpecification().MainName.data());
		if (m_Specification.GeometryShader) generateShaderCreateInfo(VK_SHADER_STAGE_GEOMETRY_BIT, api_cast<VulkanShader*>(m_Specification.GeometryShader)->GetVkShaderModule(), m_Specification.GeometryShader->GetSpecification().MainName.data());
		if (m_Specification.FragmentShader) generateShaderCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, api_cast<VulkanShader*>(m_Specification.FragmentShader)->GetVkShaderModule(), m_Specification.FragmentShader->GetSpecification().MainName.data());
		
		VulkanInputLayout& vulkanInputLayout = *api_cast<VulkanInputLayout*>(m_Specification.Input);
		const auto& bindingDescriptions = vulkanInputLayout.GetBindingDescriptions();
		const auto& attributeDescriptions = vulkanInputLayout.GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexBindingDescriptions = (!bindingDescriptions.empty() ? bindingDescriptions.data() : nullptr);
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = (!attributeDescriptions.empty() ? attributeDescriptions.data() : nullptr);

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = PrimitiveTypeToVkPrimitiveTopology(m_Specification.Primitive);
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState = {}; // Note: We currently only support 1 viewport/scissor
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		//rasterizer.depthClampEnable = VK_FALSE;
		//rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = RasterFillModeToVkPolygonMode(rasterState.FillMode);
		rasterizer.cullMode = RasterCullingModeToVkCullModeFlags(rasterState.CullingMode);
		rasterizer.frontFace = (rasterState.FrontCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE);
		rasterizer.depthBiasEnable = rasterState.DepthBias;
		rasterizer.depthBiasConstantFactor = static_cast<float>(rasterState.DepthBias);
		rasterizer.depthBiasClamp = static_cast<float>(rasterState.DepthBiasClamp);
		rasterizer.depthBiasSlopeFactor = static_cast<float>(rasterState.SlopeScaledDepthBias);
		rasterizer.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = blendState.AlphaToCoverageEnable;
		multisampling.rasterizationSamples = static_cast<VkSampleCountFlagBits>(SampleCountToVkSampleCountFlags(vulkanFramebuffer.GetSpecification().ColourAttachment.ImagePtr->GetSpecification().SampleCount));

		VkPipelineColorBlendAttachmentState colourBlendAttachment = {}; // Note: We currently only support 1 colour attachment
		colourBlendAttachment.blendEnable = blendState.Target.BlendEnable;
		colourBlendAttachment.srcColorBlendFactor = BlendFactorToVkBlendFactor(blendState.Target.SrcBlend);
		colourBlendAttachment.dstColorBlendFactor = BlendFactorToVkBlendFactor(blendState.Target.DstBlend);
		colourBlendAttachment.colorBlendOp = BlendOperationToVkBlendOp(blendState.Target.BlendOp);
		colourBlendAttachment.srcAlphaBlendFactor = BlendFactorToVkBlendFactor(blendState.Target.SrcBlendAlpha);
		colourBlendAttachment.dstAlphaBlendFactor = BlendFactorToVkBlendFactor(blendState.Target.DstBlendAlpha);
		colourBlendAttachment.alphaBlendOp = BlendOperationToVkBlendOp(blendState.Target.BlendOpAlpha);
		colourBlendAttachment.colorWriteMask = ColourMaskToVkColorComponentFlags(blendState.Target.ColourWriteMask);

		VkPipelineColorBlendStateCreateInfo colorBlending = {}; // Note: We currently only support 1 colour attachment
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colourBlendAttachment;

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = depthStencilState.DepthTestEnable;
		depthStencil.depthWriteEnable = depthStencilState.DepthWriteEnable;
		depthStencil.depthCompareOp = ComparisonFuncToVkCompareOp(depthStencilState.DepthFunc);
		//depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = depthStencilState.StencilEnable;
		//depthStencil.minDepthBounds = 0.0f; // Optional
		//depthStencil.maxDepthBounds = 1.0f; // Optional

		depthStencil.front.passOp = StencilOperationToVkStencilOp(depthStencilState.FrontFaceStencil.PassOp);
		depthStencil.front.depthFailOp = StencilOperationToVkStencilOp(depthStencilState.FrontFaceStencil.DepthFailOp);
		depthStencil.front.compareOp = ComparisonFuncToVkCompareOp(depthStencilState.FrontFaceStencil.StencilFunc);
		depthStencil.front.compareMask = depthStencilState.StencilReadMask;
		depthStencil.front.writeMask = depthStencilState.StencilWriteMask;
		depthStencil.front.reference = depthStencilState.StencilRefValue;

		depthStencil.back.passOp = StencilOperationToVkStencilOp(depthStencilState.BackFaceStencil.PassOp);
		depthStencil.back.depthFailOp = StencilOperationToVkStencilOp(depthStencilState.BackFaceStencil.DepthFailOp);
		depthStencil.back.compareOp = ComparisonFuncToVkCompareOp(depthStencilState.BackFaceStencil.StencilFunc);
		depthStencil.back.compareMask = depthStencilState.StencilReadMask;
		depthStencil.back.writeMask = depthStencilState.StencilWriteMask;
		depthStencil.back.reference = depthStencilState.StencilRefValue;

		std::vector<VkDynamicState> dynamicStates = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		// Pipeline layout
		CreatePipelineLayout(m_PipelineLayout, m_Specification.BindingLayouts, vulkanDevice.GetContext().GetVulkanLogicalDevice().GetVkDevice(), m_PushConstantsStage);

		// Create the actual graphics pipeline (where we actually use the shaders and other info)
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = nullptr; // Note: Put dynamic render info here when we need it/want to support it.
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.renderPass = vulkanRenderpass.GetVkRenderPass();
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		VK_VERIFY(vkCreateGraphicsPipelines(vulkanDevice.GetContext().GetVulkanLogicalDevice().GetVkDevice(), vulkanDevice.GetAllocator().GetPipelineCache(), 1, &pipelineInfo, VulkanAllocator::GetCallbacks(), &m_Pipeline));
		
		if constexpr (Information::Validation)
		{
			if (!m_Specification.DebugName.empty())
			{
				vulkanDevice.GetContext().SetDebugName(m_PipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, std::format("PipelineLayout for: {0}", m_Specification.DebugName));
				vulkanDevice.GetContext().SetDebugName(m_Pipeline, VK_OBJECT_TYPE_PIPELINE, std::string(m_Specification.DebugName));
			}
		}
	}

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
    {
    }

	////////////////////////////////////////////////////////////////////////////////////
	// VulkanComputePipeline
	////////////////////////////////////////////////////////////////////////////////////
	VulkanComputePipeline::VulkanComputePipeline(const Device& device, const ComputePipelineSpecification& specs)
		: m_Specification(specs)
	{
		NG_ASSERT(specs.ComputeShader, "[VkComputePipeline] No compute shader passed in to compute pipeline.");

		const VulkanDevice& vulkanDevice = *api_cast<const VulkanDevice*>(&device);

		VkPipelineShaderStageCreateInfo computeShaderInfo = {};
		computeShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderInfo.module = api_cast<const VulkanShader*>(specs.ComputeShader)->GetVkShaderModule();
		computeShaderInfo.pName = specs.ComputeShader->GetSpecification().MainName.data();

		CreatePipelineLayout(m_PipelineLayout, m_Specification.BindingLayouts, vulkanDevice.GetContext().GetVulkanLogicalDevice().GetVkDevice(), m_PushConstantsStage);

		// Create the actual compute pipeline (where we actually use the shaders and other info)
		VkComputePipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = computeShaderInfo;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VK_VERIFY(vkCreateComputePipelines(vulkanDevice.GetContext().GetVulkanLogicalDevice().GetVkDevice(), vulkanDevice.GetAllocator().GetPipelineCache(), 1, &pipelineInfo, VulkanAllocator::GetCallbacks(), &m_Pipeline));
	
		if constexpr (Information::Validation)
		{
			if (!m_Specification.DebugName.empty())
			{
				vulkanDevice.GetContext().SetDebugName(m_PipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, std::format("PipelineLayout for: {0}", m_Specification.DebugName));
				vulkanDevice.GetContext().SetDebugName(m_Pipeline, VK_OBJECT_TYPE_PIPELINE, m_Specification.DebugName);
			}
		}
	}

	VulkanComputePipeline::~VulkanComputePipeline()
	{
	}

}