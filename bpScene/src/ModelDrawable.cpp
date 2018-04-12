#include <bpScene/ModelDrawable.h>

namespace bpScene
{

void ModelDrawable::init(bp::GraphicsPipeline& pipeline, ModelResources& model)
{
	ModelDrawable::pipeline = &pipeline;
	ModelDrawable::model = &model;
}

void ModelDrawable::draw(VkCommandBuffer cmdBuffer)
{
	for (unsigned i = 0; i < model->getMeshCount(); i++)
	{
		auto& material = model->getMaterialForMesh(i);
		VkDescriptorSet set = material.getDescriptorSet();
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
					pipeline->getPipelineLayout(), 0, 1, &set, 0, nullptr);

		auto& mesh = model->getMesh(i);
		mesh.bind(cmdBuffer);
		vkCmdDrawIndexed(cmdBuffer, mesh.getElementCount(), 1, 0, 0, 0);
	}
}

}