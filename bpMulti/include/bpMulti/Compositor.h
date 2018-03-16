#ifndef BP_COMPOSITOR_H
#define BP_COMPOSITOR_H

#include <bp/Renderer.h>
#include <bp/Shader.h>
#include <bp/DescriptorSetLayout.h>
#include <bp/PipelineLayout.h>
#include <bp/GraphicsPipeline.h>
#include <bp/DescriptorPool.h>
#include <vector>
#include "Contribution.h"
#include "RenderDeviceSteps.h"
#include "CompositingDrawable.h"

namespace bpMulti
{

class Compositor : public bp::Renderer
{
public:
	virtual ~Compositor() = default;

protected:
	bp::Shader vertexShader, fragmentShader;
	bp::DescriptorSetLayout descriptorSetLayout;
	bp::PipelineLayout pipelineLayout;
	bp::GraphicsPipeline pipeline;
	bp::DescriptorPool descriptorPool;

	RenderDeviceSteps primaryRenderDeviceSteps;
	std::vector<Contribution> primaryContributions;

	std::vector<RenderDeviceSteps> secondaryRenderDeviceSteps;
	std::vector<Contribution> secondaryContributions;

	virtual void hostCopyStep() = 0;
	virtual void hostToDeviceStep(VkCommandBuffer cmdBuffer) = 0;
};

}

#endif