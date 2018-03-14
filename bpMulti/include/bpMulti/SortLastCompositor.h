#ifndef BP_SORTLASTCOMPOSITOR_H
#define BP_SORTLASTCOMPOSITOR_H

#include "Compositor.h"
#include "SortLastRenderer.h"
#include <bp/Shader.h>
#include <bp/DescriptorSetLayout.h>
#include <bp/PipelineLayout.h>
#include <bp/GraphicsPipeline.h>
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
	void init(std::initializer_list<std::pair<bp::Device*, SortLastRenderer*>> configurations,
		  VkFormat colorFormat, uint32_t width, uint32_t height);
	void resize(uint32_t width, uint32_t height) override;
	void render(bp::Framebuffer& fbo, VkCommandBuffer cmdBuffer) override;
private:
	bp::Shader vertexShader, fragmentShader;
	bp::DescriptorSetLayout descriptorSetLayout;
	bp::PipelineLayout pipelineLayout;
	bp::GraphicsPipeline pipeline;

	bpScene::DrawableSubpass subpass;
	std::vector<SortLastCompositingDrawable> compositingDrawables;

	RenderStep primaryRenderStep;

	std::vector<bp::Device*> secondaryDevices;
	std::vector<RenderStep> secondaryRenderSteps;
	std::vector<DeviceToHostStep> deviceToHostSteps;
	std::vector<HostCopyStep> hostCopySteps;
	std::vector<HostToDeviceStep> hostToDeviceSteps;

	void setupSubpasses() override;
	void initResources(uint32_t width, uint32_t height) override;
	void initShaders();
	void initDescriptorSetLayout();
	void initPipelineLayout();
	void initPipeline();
};

}

#endif