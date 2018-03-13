#ifndef BP_DRAWABLE_H
#define BP_DRAWABLE_H

#include <bp/GraphicsPipeline.h>
#include <bpUtil/Event.h>

namespace bpScene
{

class Drawable
{
public:
	virtual ~Drawable() = default;

	virtual void draw(VkCommandBuffer cmdBuffer) = 0;

	virtual bp::GraphicsPipeline* getPipeline() { return nullptr; }

	bpUtil::Event<VkCommandBuffer> resourceBindingEvent;
};

}

#endif