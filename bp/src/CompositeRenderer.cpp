#include <bp/CompositeRenderer.h>
#include <stdexcept>
#include "composite.vert.spv.h"
#include "composite.frag.spv.h"

using namespace std;

namespace bp
{

CompositeRenderer::CompositeRenderer(RenderPass& renderPass, const std::vector<VkRect2D>& areas) :
	Renderer{renderPass},
	device{renderPass.getRenderTarget().getDevice()}
{
	uint32_t i = 0;
	for (auto& area : areas)
	{
		sources.emplace_back(device, area, i++);
	}

	createShaders();
	createDescriptorPool();
	createDescriptorSetLayout();
	createDescriptorSets();
	setupDescriptors();
	createPipelineLayout();
	createPipeline();
}

CompositeRenderer::~CompositeRenderer()
{
	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	for (auto descriptor : imageDescriptors) delete descriptor;
	for (auto set : descriptorSets) delete set;
	delete descriptorSetLayout;
	delete descriptorPool;
	delete fragmentShader;
	delete vertexShader;
}

void CompositeRenderer::render(VkSemaphore waitSem)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmdBuffer, &beginInfo);

	for (auto& src : sources)
	{
		src.image.transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				     VK_ACCESS_SHADER_READ_BIT,
				     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, cmdBuffer);
	}

	renderPass.getRenderTarget().beginFrame(cmdBuffer);
	renderPass.begin(cmdBuffer);
	draw(cmdBuffer);
	renderPass.end(cmdBuffer);
	renderPass.getRenderTarget().endFrame(cmdBuffer);

	vkEndCommandBuffer(cmdBuffer);

	VkPipelineStageFlags waitStages = {VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderCompleteSem;

	if (waitSem != VK_NULL_HANDLE)
	{
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &waitSem;
	}

	submitInfo.pWaitDstStageMask = &waitStages;

	Queue& queue = renderPass.getRenderTarget().getDevice().getGraphicsQueue();
	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);
}

void CompositeRenderer::draw(VkCommandBuffer)
{
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	const VkRect2D& area = renderPass.getRenderArea();
	VkViewport viewport = {(float) area.offset.x, (float) area.offset.y,
			       (float) area.extent.width, (float) area.extent.height, 0.f, 1.f};
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
	vkCmdSetScissor(cmdBuffer, 0, 1, &area);

	for (auto i = 0; i < sources.size(); i++)
	{
		VkDescriptorSet set = *descriptorSets[i];
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
					0, 1, &set, 0, nullptr);

		vkCmdPushConstants(cmdBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
				   sizeof(PushConstant), &sources[i].pushConstant);
		vkCmdDraw(cmdBuffer, 4, 1, 0, 0);
	}
}

CompositeRenderer::CompositeSource::CompositeSource(bp::Device& device, const VkRect2D& area,
						    uint32_t samplerIndex) :
	device{device},
	image{device, area.extent.width, area.extent.height, VK_FORMAT_R8G8B8A8_UNORM,
	      VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT,
	      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT}
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image.getHandle();
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &imageView);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create image view.");

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	result = vkCreateSampler(device, &samplerInfo, nullptr, &sampler);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create image sampler.");

	uint32_t width = area.extent.width;
	uint32_t height = area.extent.height;

	pushConstant.x = 2.f * static_cast<float>(area.offset.x) / static_cast<float>(width) - 1.f;
	pushConstant.y = 2.f * static_cast<float>(area.offset.y) / static_cast<float>(height) - 1.f;
	pushConstant.w = 2.f * static_cast<float>(area.extent.width) / static_cast<float>(width);
	pushConstant.h = 2.f * static_cast<float>(area.extent.height) / static_cast<float>(height);
	pushConstant.samplerIndex = samplerIndex;
}

CompositeRenderer::CompositeSource::~CompositeSource()
{
	vkDestroySampler(device, sampler, nullptr);
	vkDestroyImageView(device, imageView, nullptr);
}

void CompositeRenderer::createShaders()
{
	vertexShader = new Shader(device, VK_SHADER_STAGE_VERTEX_BIT,
				  (uint32_t) sizeof(vert_spv),
				  (const uint32_t*) vert_spv);

	fragmentShader = new Shader(device, VK_SHADER_STAGE_FRAGMENT_BIT,
				    (uint32_t) sizeof(frag_spv),
				    (const uint32_t*) frag_spv);
}

void CompositeRenderer::createDescriptorPool()
{
	descriptorPool = new DescriptorPool(
		device,
		{{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, sources.size()}},
		sources.size());
}

void CompositeRenderer::createDescriptorSetLayout()
{
	descriptorSetLayout = new DescriptorSetLayout(
		device,
		{{
			 0,
			 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			 1,
			 VK_SHADER_STAGE_FRAGMENT_BIT,
			 nullptr}
		}
	);
}

void CompositeRenderer::createDescriptorSets()
{
	descriptorSets.resize(sources.size());
	for (auto i = 0; i < sources.size(); i++)
	{
		descriptorSets[i] = new DescriptorSet(device, *descriptorPool,
						      *descriptorSetLayout);
	}
}

void CompositeRenderer::setupDescriptors()
{
	imageDescriptors.resize(sources.size());
	for (auto i = 0; i < sources.size(); i++)
	{
		imageDescriptors[i] =
			new ImageDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, 0,
					    {{
						     sources[i].sampler,
						     sources[i].imageView,
						     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
					     }});
		descriptorSets[i]->bind(imageDescriptors[i]);
		descriptorSets[i]->update();
	}
}

void CompositeRenderer::createPipelineLayout()
{
	VkPushConstantRange pushConstantRange = {VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t)};

	VkDescriptorSetLayout setLayout = *descriptorSetLayout;

	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.setLayoutCount = 1;
	info.pSetLayouts = &setLayout;
	info.pushConstantRangeCount = 1;
	info.pPushConstantRanges = &pushConstantRange;

	VkResult result = vkCreatePipelineLayout(device, &info, nullptr, &pipelineLayout);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create pipeline layout.");
}

void CompositeRenderer::createPipeline()
{
	vector<VkPipelineShaderStageCreateInfo> shaderStages;
	shaderStages.push_back(vertexShader->getPipelineShaderStageInfo());
	shaderStages.push_back(fragmentShader->getPipelineShaderStageInfo());

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
	vertexInputStateCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
	inputAssemblyStateCreateInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
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
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

	if (renderPass.getRenderTarget().isDepthImageEnabled())
	{
		depthState.depthTestEnable = VK_TRUE;
		depthState.depthWriteEnable = VK_TRUE;
	}

	depthState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthState.front = noOPStencilState;
	depthState.back = noOPStencilState;

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
	pipelineCreateInfo.stageCount = (uint32_t) shaderStages.size();
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
	pipelineCreateInfo.pTessellationState = nullptr;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pDepthStencilState = &depthState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = renderPass.getHandle();
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = nullptr;
	pipelineCreateInfo.basePipelineIndex = 0;

	VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo,
						    nullptr, &pipeline);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create pipeline.");
}

}