#ifndef BP_SHADER_H
#define BP_SHADER_H

#include <vulkan/vulkan.h>

namespace bp
{

class Shader
{
public:
	Shader(VkDevice device, VkShaderStageFlagBits stage, uint32_t codeSize,
	       const uint32_t* code);
	~Shader();

	VkShaderStageFlags getStage() const { return stage; }
	VkShaderModule getHandle() { return handle; }
	const VkPipelineShaderStageCreateInfo& getPipelineShaderStageInfo() const
	{
		return pipelineShaderStageInfo;
	}

private:
	VkDevice device;
	VkShaderStageFlagBits stage;
	VkShaderModule handle;
	VkPipelineShaderStageCreateInfo pipelineShaderStageInfo;
};

}

#endif