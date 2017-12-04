#include <bp/ComputePipeline.h>
#include <stdexcept>

using namespace std;

namespace bp
{

void ComputePipeline::create()
{
	if (shaderStageInfos.size() != 1)
		throw runtime_error("A singe shader stage must be added to a compute pipeline.");

	VkComputePipelineCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	info.stage = shaderStageInfos[0];
	info.layout = layout;

	VkResult result = vkCreateComputePipelines(*device, VK_NULL_HANDLE, 1, &info, nullptr,
						   &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create compute pipeline.");
}

}