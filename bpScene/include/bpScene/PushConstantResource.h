#ifndef BP_SCENERESOURCE_H
#define BP_SCENERESOURCE_H

#include "Math.h"
#include "Node.h"
#include "Camera.h"
#include <vulkan/vulkan.h>

namespace bpScene
{

class PushConstantResource
{
public:
	struct Matrices
	{
		glm::mat4 mvpMatrix;
		glm::mat4 normalMatrix;
	};

	PushConstantResource() :
		pipelineLayout{VK_NULL_HANDLE},
		shaderStage{VK_NULL_HANDLE},
		node{nullptr},
		camera{nullptr} {}
	PushConstantResource(VkPipelineLayout pipelineLayout, VkShaderStageFlags shaderStage,
			     bpScene::Node& node, bpScene::Camera& camera) :
		PushConstantResource{}
	{
		init(pipelineLayout, shaderStage, node, camera);
	}

	void init(VkPipelineLayout pipelineLayout, VkShaderStageFlags shaderStage,
		  bpScene::Node& node, bpScene::Camera& camera);
	void bind(VkCommandBuffer cmdBuffer);

	void setNode(bpScene::Node& node) { PushConstantResource::node = &node; }
	void setCamera(bpScene::Camera& camera) { PushConstantResource::camera = &camera; }

	const Matrices& getMatrices() const { return matrices; }

private:
	VkPipelineLayout pipelineLayout;
	VkShaderStageFlags shaderStage;

	bpScene::Node* node;
	bpScene::Camera* camera;

	Matrices matrices;

	void update();
};

}

#endif