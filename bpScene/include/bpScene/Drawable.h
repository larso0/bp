#ifndef BP_DRAWABLE_H
#define BP_DRAWABLE_H

#include <bp/GraphicsPipeline.h>
#include <bpUtil/Event.h>

namespace bpScene
{

class Drawable
{
public:
	Drawable() :
		pipeline{nullptr} {}
	virtual ~Drawable() = default;

	virtual void draw(VkCommandBuffer cmdBuffer) = 0;

	bp::GraphicsPipeline* getPipeline() { return pipeline; }

	bpUtil::Event<VkCommandBuffer> resourceBindingEvent;

protected:
	bp::GraphicsPipeline* pipeline;
};

}

#endif