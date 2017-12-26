#include <bp/GraphicsPipeline.h>
#include <stdexcept>

using namespace std;

namespace bp
{

void GraphicsPipeline::create()
{
	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
	vertexInputStateCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount =
		static_cast<uint32_t>(vertexBindingDescriptions.size());
	vertexInputStateCreateInfo.pVertexBindingDescriptions = vertexBindingDescriptions.data();
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount =
		static_cast<uint32_t>(vertexAttributeDescriptions.size());
	vertexInputStateCreateInfo.pVertexAttributeDescriptions =
		vertexAttributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
	inputAssemblyStateCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCreateInfo.topology = primitiveTopology;
	inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport vp = {};
	VkRect2D sc = {};

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &vp;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &sc;

	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType =
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.polygonMode = polygonMode;
	rasterizationState.cullMode = cullMode;
	rasterizationState.frontFace = frontFace;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.depthBiasConstantFactor = 0;
	rasterizationState.depthBiasClamp = 0;
	rasterizationState.depthBiasSlopeFactor = 0;
	rasterizationState.lineWidth = 1;

	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType =
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.sampleShadingEnable = VK_FALSE;
	multisampleState.minSampleShading = 0;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = VK_FALSE;
	multisampleState.alphaToOneEnable = VK_FALSE;

	VkStencilOpState noOPStencilState = {};
	noOPStencilState.failOp = VK_STENCIL_OP_KEEP;
	noOPStencilState.passOp = VK_STENCIL_OP_KEEP;
	noOPStencilState.depthFailOp = VK_STENCIL_OP_KEEP;
	noOPStencilState.compareOp = VK_COMPARE_OP_ALWAYS;
	noOPStencilState.compareMask = 0;
	noOPStencilState.writeMask = 0;
	noOPStencilState.reference = 0;

	VkPipelineDepthStencilStateCreateInfo depthState = {};
	depthState.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	if (depthEnabled)
	{
		depthState.depthTestEnable = VK_TRUE;
		depthState.depthWriteEnable = VK_TRUE;
	}
	depthState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthState.depthBoundsTestEnable = VK_FALSE;
	depthState.stencilTestEnable = VK_FALSE;
	depthState.front = noOPStencilState;
	depthState.back = noOPStencilState;
	depthState.minDepthBounds = 0;
	depthState.maxDepthBounds = 0;

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
	colorBlendAttachmentState.blendEnable = VK_FALSE;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
	colorBlendAttachmentState.dstColorBlendFactor =
		VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.colorWriteMask = 0xf;

	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType =
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp = VK_LOGIC_OP_CLEAR;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachmentState;
	colorBlendState.blendConstants[0] = 0.0;
	colorBlendState.blendConstants[1] = 0.0;
	colorBlendState.blendConstants[2] = 0.0;
	colorBlendState.blendConstants[3] = 0.0;

	VkDynamicState dynamicState[2] = {VK_DYNAMIC_STATE_VIEWPORT,
					  VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = 2;
	dynamicStateCreateInfo.pDynamicStates = dynamicState;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStageInfos.size());
	pipelineCreateInfo.pStages = shaderStageInfos.data();
	pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
	pipelineCreateInfo.pTessellationState = nullptr;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pDepthStencilState = &depthState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.layout = layout;
	pipelineCreateInfo.renderPass = *renderPass;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = nullptr;
	pipelineCreateInfo.basePipelineIndex = 0;

	VkResult result = vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &pipelineCreateInfo,
						    nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create graphics pipeline.");
}

}