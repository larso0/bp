#ifndef BP_RENDERER_H
#define BP_RENDERER_H

#include "RenderPass.h"

namespace bp
{

class Renderer
{
public:
	explicit Renderer(RenderPass& renderPass) : renderPass{renderPass} {}
	virtual ~Renderer() = default;

	virtual void update(float delta) {}
	virtual void render(VkCommandBuffer cmdBuffer) = 0;

protected:
	RenderPass& renderPass;
};

}

#endif