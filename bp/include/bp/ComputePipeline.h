#ifndef BP_COMPUTEPIPELINE_H
#define BP_COMPUTEPIPELINE_H

#include "Pipeline.h"

namespace bp
{

class ComputePipeline : public Pipeline
{
public:
	ComputePipeline() : Pipeline{} {}
	ComputePipeline(NotNull<Device> device, NotNull<RenderPass> renderPass,
			VkPipelineLayout layout,
			std::initializer_list<VkPipelineShaderStageCreateInfo> shaderStageInfos) :
		Pipeline{device, renderPass, layout, shaderStageInfos} {}
private:
	void create() override;
};

}

#endif