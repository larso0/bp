#ifndef BP_COMPOSITOR_H
#define BP_COMPOSITOR_H

#include <bp/Renderer.h>
#include <bp/Shader.h>
#include <bp/DescriptorSetLayout.h>
#include <bp/PipelineLayout.h>
#include <bp/GraphicsPipeline.h>
#include <bp/DescriptorPool.h>
#include <bpScene/DrawableSubpass.h>
#include <vector>
#include "Contribution.h"
#include "RenderDeviceSteps.h"
#include "CompositingDrawable.h"

namespace bpMulti
{

class Compositor : public bp::Renderer
{
public:
	Compositor() :
		transferQueue{nullptr},
		transferCommandBuffer{VK_NULL_HANDLE},
		deviceCount{1}, currentFrameIndex{0},
		primaryRenderer{nullptr} {}
	virtual ~Compositor() = default;

	void render(bp::Framebuffer& fbo, VkCommandBuffer cmdBuffer) override;
	void renderFirstFrame();
protected:
	bp::Shader vertexShader, fragmentShader;
	bp::DescriptorSetLayout descriptorSetLayout;
	bp::PipelineLayout pipelineLayout;
	bp::GraphicsPipeline pipeline;
	bp::DescriptorPool descriptorPool;
	bp::Queue* transferQueue;
	bp::CommandPool transferCommandPool;
	VkCommandBuffer transferCommandBuffer;

	unsigned deviceCount;
	unsigned currentFrameIndex;

	bp::Renderer* primaryRenderer;
	RenderDeviceSteps primaryRenderDeviceSteps;
	std::vector<Contribution> primaryContributions;

	std::vector<bp::Device*> secondaryDevices;
	std::vector<bp::Renderer*> secondaryRenderers;

	std::vector<RenderDeviceSteps> secondaryRenderDeviceSteps;
	std::vector<Contribution> secondaryContributions;

	bpScene::DrawableSubpass subpass;
	std::vector<CompositingDrawable> drawables;

	virtual VkExtent2D getContributionSize(unsigned deviceIndex) = 0;
	virtual unsigned getCompositingElementCount() const = 0;
	virtual bool shouldCopyDepth() const = 0;
	void hostCopyStep();
	void hostToDeviceStep();
	virtual void initShaders() = 0;
	virtual void initDescriptorSetLayout() = 0;
	virtual void initPipelineLayout() = 0;
	virtual void initPipeline() = 0;
	virtual void initDescriptorPool() = 0;
	virtual void setupContribution(Contribution& contribution) = 0;
	void setupSubpasses() override;
	void initResources(uint32_t width, uint32_t height) override;
};

}

#endif