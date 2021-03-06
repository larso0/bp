#include <bpScene/DrawableSubpass.h>
#include <algorithm>

using namespace bp;
using namespace std;

namespace bpScene
{

void DrawableSubpass::render(const VkRect2D& area, VkCommandBuffer cmdBuffer)
{
	VkViewport viewport = {(float) area.offset.x, (float) area.offset.y,
			       (float) area.extent.width, (float) area.extent.height, 0.f, 1.f};

	GraphicsPipeline* currentPipeline = nullptr;
	for (auto d : drawables)
	{
		if (d->getPipeline() != currentPipeline)
		{
			currentPipeline = d->getPipeline();
			if (currentPipeline != nullptr)
			{
				vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
						  *currentPipeline);
				vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
				vkCmdSetScissor(cmdBuffer, 0, 1, &area);
			}
		}
		d->resourceBindingEvent(cmdBuffer);
		d->draw(cmdBuffer);
	}
}

static bool drawablePipelineSortPredicate(Drawable* a, Drawable* b)
{
	return a->getPipeline() < b->getPipeline();
}

void DrawableSubpass::addDrawable(Drawable& drawable)
{
	drawables.insert(upper_bound(drawables.begin(), drawables.end(), &drawable,
				     drawablePipelineSortPredicate),
			 &drawable);
}

void DrawableSubpass::removeDrawable(Drawable& drawable)
{
	auto lower = lower_bound(drawables.begin(), drawables.end(), &drawable,
				 drawablePipelineSortPredicate);
	if (lower != drawables.end())
	{
		auto upper = upper_bound(lower, drawables.end(), &drawable,
					 drawablePipelineSortPredicate);
		auto pos = find(lower, upper, &drawable);
		if (pos != upper) drawables.erase(pos);
	}
}

}