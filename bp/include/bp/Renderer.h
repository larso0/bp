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

	void render(VkSemaphore waitSem = VK_NULL_HANDLE);

	virtual void update(float delta) {}
	virtual void draw(VkCommandBuffer cmdBuffer) = 0;

	VkSemaphore getRenderCompleteSemaphore() { return renderCompleteSem; }

protected:
	RenderPass& renderPass;

	VkCommandBuffer cmdBuffer;
	VkSemaphore renderCompleteSem;
};

}

#endif