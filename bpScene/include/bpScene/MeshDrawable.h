#ifndef BP_MESHDRAWABLE_H
#define BP_MESHDRAWABLE_H

#include "Drawable.h"
#include "MeshResources.h"

namespace bpScene
{

class MeshDrawable : public Drawable
{
public:
	MeshDrawable() :
		Drawable{},
		mesh{nullptr},
		elementCount{0},
		instanceCount{1} {}
	MeshDrawable(bp::GraphicsPipeline& pipeline, MeshResources& mesh, uint32_t offset,
		     uint32_t elementCount, uint32_t instanceCount = 1) :
		MeshDrawable{}
	{
		init(pipeline, mesh, offset, elementCount, instanceCount);
	}
	virtual ~MeshDrawable() = default;

	void init(bp::GraphicsPipeline& pipeline, MeshResources& mesh, uint32_t offset,
		  uint32_t elementCount, uint32_t instanceCount = 1);

	void draw(VkCommandBuffer cmdBuffer) override;

private:
	MeshResources* mesh;
	uint32_t elementCount;
	uint32_t instanceCount;
};

}

#endif