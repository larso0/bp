#ifndef BP_DRAWABLESUBPASS_H
#define BP_DRAWABLESUBPASS_H

#include "Drawable.h"
#include <bp/Subpass.h>
#include <vector>

namespace bpScene
{

class DrawableSubpass : public bp::Subpass
{
public:
	DrawableSubpass() :
		Subpass{} {}

	void init(bp::RenderPass& renderPass) override;
	void render(VkCommandBuffer cmdBuffer) override;
	void addDrawable(Drawable* drawable);

private:
	bp::RenderPass* renderPass;
	std::vector<Drawable*> drawables;
};

}

#endif