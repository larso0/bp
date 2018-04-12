#ifndef BP_SCENE_MODELDRAWABLE_H
#define BP_SCENE_MODELDRAWABLE_H

#include "Drawable.h"
#include "ModelResources.h"

namespace bpScene
{

class ModelDrawable : public Drawable
{
public:
	ModelDrawable() : pipeline{nullptr}, model{nullptr} {}

	void init(bp::GraphicsPipeline& pipeline, ModelResources& model);
	void draw(VkCommandBuffer cmdBuffer) override;
	bp::GraphicsPipeline* getPipeline() override { return pipeline; }
private:
	bp::GraphicsPipeline* pipeline;
	ModelResources* model;
};

}

#endif