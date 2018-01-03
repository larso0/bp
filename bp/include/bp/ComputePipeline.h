#ifndef BP_COMPUTEPIPELINE_H
#define BP_COMPUTEPIPELINE_H

#include "Pipeline.h"

namespace bp
{

class ComputePipeline : public Pipeline
{
public:
	ComputePipeline() : Pipeline{} {}
	ComputePipeline(NotNull<Device> device, VkPipelineLayout layout,
			std::initializer_list<VkPipelineShaderStageCreateInfo> shaderStageInfos) :
		Pipeline{}
	{
		addShaderStageInfos(shaderStageInfos.begin(), shaderStageInfos.end());
		init(device, layout);
	}

	void init(NotNull<Device> device, VkPipelineLayout layout)
	{
		Pipeline::device = device;
		Pipeline::layout = layout;
		create();
	}
private:
	void create();
};

}

#endif