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
	CompositingDrawable() :
		pipeline{nullptr}, elementCount{3} {}

	void init(bp::GraphicsPipeline& pipeline, unsigned elementCount)
	{
		CompositingDrawable::pipeline = &pipeline;
		CompositingDrawable::elementCount = elementCount;
	}

	void draw(VkCommandBuffer cmdBuffer) override
	{
		vkCmdDraw(cmdBuffer, elementCount, 1, 0, 0);
	}

	bp::GraphicsPipeline* getPipeline() override { return pipeline; }
private:
	bp::GraphicsPipeline* pipeline;
	unsigned elementCount;
};

}

#endif