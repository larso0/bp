#include <bpScene/PushConstantResource.h>

using glm::transpose;
using glm::inverse;

namespace bpScene
{

void PushConstantResource::init(VkPipelineLayout pipelineLayout, VkPipelineStageFlags pipelineStage,
				bpScene::Node& node, bpScene::Camera& camera)
{
	PushConstantResource::pipelineLayout = pipelineLayout;
	PushConstantResource::pipelineStage = pipelineStage;
	PushConstantResource::node = &node;
	PushConstantResource::camera = &camera;
	update();
}

void PushConstantResource::update()
{
	matrices.mvpMatrix = camera->getProjectionMatrix() * camera->getViewMatrix()
			     * node->getWorldMatrix();
	matrices.normalMatrix = transpose(inverse(node->getWorldMatrix()));
}

void PushConstantResource::bind(VkCommandBuffer cmdBuffer)
{
	vkCmdPushConstants(cmdBuffer, pipelineLayout, pipelineStage, 0, sizeof(Matrices),
			   &matrices);
}

}