#ifndef BP_SORTLASTCOMPOSITOR_H
#define BP_SORTLASTCOMPOSITOR_H

#include "Compositor.h"
#include <bp/Shader.h>
#include <bp/DescriptorSetLayout.h>
#include <bp/PipelineLayout.h>
#include <bp/GraphicsPipeline.h>

namespace bpMulti
{

class SortLastCompositor : public Compositor
{
public:

private:
	bp::Shader vertexShader, fragmentShader;
	bp::DescriptorSetLayout descriptorSetLayout;
	bp::PipelineLayout pipelineLayout;
	bp::GraphicsPipeline pipeline;
	VkSampler sampler;

	void initShaders();
};

}

#endif