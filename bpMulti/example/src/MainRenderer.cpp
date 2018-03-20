#include "MainRenderer.h"
#include <bp/Util.h>

using namespace bp;
using namespace bpScene;
using namespace std;

unsigned MainRenderer::addMesh(Mesh& mesh)
{
	unsigned meshId = static_cast<unsigned int>(meshes.size());
	meshes.emplace_back(new MeshResources{getDevice(), mesh});
	return meshId;
}

unsigned MainRenderer::addEntity(unsigned meshIndex, Node& node)
{
	unsigned entityId = static_cast<unsigned int>(drawables.size());
	drawables.emplace_back(new MeshDrawable{graphicsPipeline, *meshes[meshIndex], 0,
						meshes[meshIndex]->getElementCount()});
	pushConstants.emplace_back(new PushConstantResource{pipelineLayout,
							    VK_SHADER_STAGE_VERTEX_BIT, node,
							    *camera});
	bpUtil::connect(drawables[entityId]->resourceBindingEvent, *pushConstants[entityId],
			&PushConstantResource::bind);
	subpass.addDrawable(*drawables[entityId]);
	return entityId;
}

void MainRenderer::setupSubpasses()
{
	subpass.addColorAttachment(getColorAttachmentSlot());
	subpass.setDepthAttachment(getDepthAttachmentSlot());
	addSubpassGraph(subpass);
}

void MainRenderer::initResources(uint32_t width, uint32_t height)
{
	auto vertexShaderCode = readBinaryFile("spv/basic.vert.spv");
	vertexShader.init(getDevice(), VK_SHADER_STAGE_VERTEX_BIT,
			  static_cast<uint32_t>(vertexShaderCode.size()),
			  reinterpret_cast<const uint32_t*>(vertexShaderCode.data()));
	auto fragmentShaderCode = readBinaryFile("spv/basic.frag.spv");
	fragmentShader.init(getDevice(), VK_SHADER_STAGE_FRAGMENT_BIT,
			    static_cast<uint32_t>(fragmentShaderCode.size()),
			    reinterpret_cast<const uint32_t*>(fragmentShaderCode.data()));

	pipelineLayout.addPushConstantRange({VK_SHADER_STAGE_VERTEX_BIT, 0,
					     sizeof(PushConstantResource::Matrices)});
	pipelineLayout.init(getDevice());

	graphicsPipeline.addShaderStageInfo(vertexShader.getPipelineShaderStageInfo());
	graphicsPipeline.addShaderStageInfo(fragmentShader.getPipelineShaderStageInfo());
	graphicsPipeline.addVertexBindingDescription({0, Vertex::STRIDE,
						      VK_VERTEX_INPUT_RATE_VERTEX});
	graphicsPipeline.addVertexAttributeDescription({0, 0, VK_FORMAT_R32G32B32_SFLOAT,
							Vertex::POSITION_OFFSET});
	graphicsPipeline.addVertexAttributeDescription({1, 0, VK_FORMAT_R32G32B32_SFLOAT,
							Vertex::NORMAL_OFFSET});
	graphicsPipeline.init(getDevice(), getRenderPass(), pipelineLayout);
}
