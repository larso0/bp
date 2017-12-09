#ifndef BP_RENDERER_H
#define BP_RENDERER_H

#include "RenderTarget.h"
#include "Pointer.h"

namespace bp
{

class Renderer
{
public:
	virtual ~Renderer() = default;
	virtual void init(NotNull<RenderTarget> target, const VkRect2D& area) = 0;
	virtual void render(VkCommandBuffer cmdBuffer) = 0;
	virtual void resize(uint32_t w, uint32_t h) {}
};

}

#endif