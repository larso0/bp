#ifndef BPMULTITEST_MAINRENDERER_H
#define BPMULTITEST_MAINRENDERER_H

#include <bp/Shader.h>
#include <bp/PipelineLayout.h>
#include <bp/GraphicsPipeline.h>
#include <bpScene/DrawableSubpass.h>
#include <bpScene/MeshDrawable.h>
#include <bpScene/PushConstantResource.h>
#include <bpMulti/SortLastRenderer.h>
#include <memory>

class MainRenderer : public bpMulti::SortLastRenderer
{
public:
	MainRenderer() :
		camera{nullptr} {}

	void setCamera(bpScene::Camera& camera) { MainRenderer::camera = &camera; }
	unsigned addMesh(bpScene::Mesh& mesh);
	unsigned addEntity(unsigned meshIndex, bpScene::Node& node);

	void increaseWorkload(float hint) override {}
	void decreaseWorkload(float hint) override {}

private:
	bpScene::Camera* camera;
	bpScene::DrawableSubpass subpass;

	bp::Shader vertexShader, fragmentShader;
	bp::PipelineLayout pipelineLayout;
	bp::GraphicsPipeline graphicsPipeline;

	std::vector<std::unique_ptr<bpScene::MeshResources>> meshes;
	std::vector<std::unique_ptr<bpScene::MeshDrawable>> drawables;
	std::vector<std::unique_ptr<bpScene::PushConstantResource>> pushConstants;

	void setupSubpasses() override;
	void initResources(uint32_t width, uint32_t height) override;
};

#endif
