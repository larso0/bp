#ifndef BP_COMPOSITINGDRAWABLE_H
#define BP_COMPOSITINGDRAWABLE_H

#include "Contribution.h"
#include <bpScene/Drawable.h>
#include <bp/PipelineLayout.h>

namespace bpMulti
{

class CompositingDrawable : public bpScene::Drawable
{
public:
	struct PushConstants
	{
		float x, y; //Offset
		float w, h; //Extent
	};

	CompositingDrawable() :
		pipeline{nullptr},
		pipelineLayout{nullptr},
		pushConstants{0.f, 0.f, 1.f, 1.f} {}

	void init(bp::GraphicsPipeline& pipeline, bp::PipelineLayout& pipelineLayout)
	{
		CompositingDrawable::pipeline = &pipeline;
		CompositingDrawable::pipelineLayout = &pipelineLayout;
	}

	void setPushConstants(float x, float y, float w, float h) { pushConstants = {x, y, w, h}; }

	void draw(VkCommandBuffer cmdBuffer) override
	{
		vkCmdPushConstants(cmdBuffer, *pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
				   sizeof(PushConstants), &pushConstants);
		vkCmdDraw(cmdBuffer, 4, 1, 0, 0);
	}

	bp::GraphicsPipeline* getPipeline() override { return pipeline; }
private:
	bp::GraphicsPipeline* pipeline;
	bp::PipelineLayout* pipelineLayout;
	PushConstants pushConstants;
};

}

#endif