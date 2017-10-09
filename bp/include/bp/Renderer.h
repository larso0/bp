#ifndef BP_RENDERER_H
#define BP_RENDERER_H

#include "RenderPass.h"

namespace bp
{

class Renderer
{
public:
	Renderer() :
		renderPass(nullptr) {}
	virtual ~Renderer() = default;

	virtual void init() = 0;
	virtual void update(float delta) {}
	virtual void render(VkCommandBuffer cmdBuffer) = 0;
	virtual bool isReady() const = 0;

	void setRenderPass(RenderPass* renderPass);

protected:
	RenderPass* renderPass;
};

}

#endif