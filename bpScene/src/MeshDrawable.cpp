#include <bpScene/MeshDrawable.h>

namespace bpScene
{

void MeshDrawable::init(bp::GraphicsPipeline& pipeline, MeshResources& mesh, uint32_t offset,
			uint32_t elementCount, uint32_t instanceCount)
{
	MeshDrawable::pipeline = &pipeline;
	MeshDrawable::mesh = &mesh;
	MeshDrawable::elementCount = elementCount;
	MeshDrawable::instanceCount = instanceCount;

	bpUtil::connect(Drawable::resourceBindingEvent, mesh, &MeshResources::bind);
}

void MeshDrawable::draw(VkCommandBuffer cmdBuffer)
{
	vkCmdDrawIndexed(cmdBuffer, elementCount, instanceCount, 0, 0, 0);
}

}
