#ifndef BP_SORTLASTCOMPOSITOR_H
#define BP_SORTLASTCOMPOSITOR_H

#include "Compositor.h"
#include "SortLastRenderer.h"
#include <bp/Shader.h>
#include <bp/DescriptorSetLayout.h>
#include <bp/PipelineLayout.h>
#include <bp/GraphicsPipeline.h>
#include <bp/DescriptorPool.h>
#include <bp/DescriptorSet.h>
#include <bpScene/DrawableSubpass.h>
#include <vector>
#include <initializer_list>
#include <utility>

namespace bpMulti
{

class SortLastCompositingDrawable : public bpScene::Drawable
{
public:
	SortLastCompositingDrawable() :
		pipeline{nullptr} {}

	void init(bp::GraphicsPipeline& pipeline)
	{
		SortLastCompositingDrawable::pipeline = &pipeline;
	}

	void draw(VkCommandBuffer cmdBuffer) override
	{
		vkCmdDraw(cmdBuffer, 3, 1, 0, 0);
	}

	bp::GraphicsPipeline* getPipeline() override { return pipeline; }

private:
	bp::GraphicsPipeline* pipeline;
};

class SortLastCompositor : public Compositor
{
public:
	SortLastCompositor() :
		deviceCount{0},
		currentFrameIndex{0} {}

	void init(std::initializer_list<std::pair<bp::Device*, SortLastRenderer*>> configurations,
		  VkFormat colorFormat, uint32_t width, uint32_t height);
	void resize(uint32_t width, uint32_t height) override;
	void render(bp::Framebuffer& fbo, VkCommandBuffer cmdBuffer) override;
private:
	unsigned deviceCount;
	bp::Shader vertexShader, fragmentShader;
	bp::DescriptorSetLayout descriptorSetLayout;
	bp::PipelineLayout pipelineLayout;
	bp::GraphicsPipeline pipeline;

	bpScene::DrawableSubpass subpass;
	std::vector<SortLastCompositingDrawable> compositingDrawables;

	bp::DescriptorPool descriptorPool;
	std::vector<bp::DescriptorSet> primaryDescriptorSets;
	RenderStep primaryRenderStep;

	std::vector<bp::Device*> secondaryDevices;
	std::vector<RenderStep> secondaryRenderSteps;
	std::vector<DeviceToHostStep> deviceToHostSteps;
	std::vector<HostCopyStep> hostCopySteps;
	HostToDeviceStep hostToDeviceStep;

	unsigned currentFrameIndex;

	void setupSubpasses() override;
	void initResources(uint32_t width, uint32_t height) override;
	void initShaders();
	void initPipelineLayout();
	void initPipeline();
};

}

#endif