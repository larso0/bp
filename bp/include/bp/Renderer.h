#ifndef BP_RENDERER_H
#define BP_RENDERER_H

#include "RenderPass.h"

namespace bp
{

class Renderer
{
public:
	explicit Renderer(RenderPass& renderPass);
	virtual ~Renderer();

	void render();

	virtual void update(float delta) {}
	virtual void draw(VkCommandBuffer cmdBuffer) = 0;

protected:
	RenderPass& renderPass;

	VkCommandBuffer cmdBuffer;
	VkSemaphore renderCompleteSem;
};

}

#endif