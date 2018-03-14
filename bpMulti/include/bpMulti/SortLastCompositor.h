#ifndef BP_SORTLASTCOMPOSITOR_H
#define BP_SORTLASTCOMPOSITOR_H

#include "Compositor.h"
#include <bp/Shader.h>
#include <bp/DescriptorSetLayout.h>
#include <bp/PipelineLayout.h>
#include <bp/GraphicsPipeline.h>
#include <bpScene/DrawableSubpass.h>

namespace bpMulti
{

class SortLastCompositeDrawable : public bpScene::Drawable
{
public:
	SortLastCompositeDrawable() :
		pipeline{nullptr} {}

	void init(bp::GraphicsPipeline& pipeline)
	{
		SortLastCompositeDrawable::pipeline = &pipeline;
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
	void resize(uint32_t width, uint32_t height) override;
	void render(bp::Framebuffer& fbo, VkCommandBuffer cmdBuffer) override;
private:
	bpScene::DrawableSubpass subpass;
	SortLastCompositeDrawable drawable;

	bp::Shader vertexShader, fragmentShader;
	bp::DescriptorSetLayout descriptorSetLayout;
	bp::PipelineLayout pipelineLayout;
	bp::GraphicsPipeline pipeline;

	void setupSubpasses() override;
	void initResources(uint32_t width, uint32_t height) override;
	void initShaders();
	void initDescriptorSetLayout();
	void initPipelineLayout();
	void initPipeline();
};

}

#endif