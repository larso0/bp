#include <bpScene/PushConstantResource.h>

using glm::transpose;
using glm::inverse;

namespace bpScene
{

void PushConstantResource::init(VkPipelineLayout pipelineLayout, VkShaderStageFlags shaderStage,
				bpScene::Node& node, bpScene::Camera& camera)
{
	PushConstantResource::pipelineLayout = pipelineLayout;
	PushConstantResource::shaderStage = shaderStage;
	PushConstantResource::node = &node;
	PushConstantResource::camera = &camera;
	update();
}

void PushConstantResource::update()
{
	matrices.mvpMatrix = clipTransform * camera->getProjectionMatrix() * camera->getViewMatrix()
			     * node->getWorldMatrix() * scaleTransform;
	matrices.normalMatrix = transpose(inverse(node->getWorldMatrix()));
}

void PushConstantResource::bind(VkCommandBuffer cmdBuffer)
{
	vkCmdPushConstants(cmdBuffer, pipelineLayout, shaderStage, 0, sizeof(Matrices),
			   &matrices);
}

}