#ifndef BP_SHADER_H
#define BP_SHADER_H

#include <vulkan/vulkan.h>
#include <string>

namespace bp
{

class Shader
{
public:
	Shader(VkDevice device, VkShaderStageFlagBits stage, const std::string& glslSource);
	Shader(VkDevice device, VkShaderStageFlagBits stage, uint32_t codeSize,
	       const uint32_t* spirvCode);
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

	void createModule(uint32_t codeSize, const uint32_t* code);
};

}

#endif