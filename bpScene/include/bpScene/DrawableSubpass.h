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

	void render(const VkRect2D& area, VkCommandBuffer cmdBuffer) override;
	void addDrawable(Drawable& drawable);
	void removeDrawable(Drawable& drawable);

private:
	std::vector<Drawable*> drawables;
};

}

#endif