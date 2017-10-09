#ifndef BP_SHADER_H
#define BP_SHADER_H

#include <vulkan/vulkan.h>

namespace bp
{

class Shader
{
public:
	Shader() :
		device(VK_NULL_HANDLE),
		stage((VkShaderStageFlagBits) 0),
		codeSize(0),
		code(nullptr),
		handle(VK_NULL_HANDLE),
		pipelineShaderStageInfo({}) {}
	~Shader();

	void init();

	void setDevice(VkDevice device);
	void setStage(VkShaderStageFlagBits stage);
	void setCode(uint32_t size, const uint32_t* code);

	VkShaderStageFlags getStage() const { return stage; }
	VkShaderModule getHandle() const { return handle; }
	const VkPipelineShaderStageCreateInfo& getPipelineShaderStageInfo() const
	{
		return pipelineShaderStageInfo;
	}
	bool isReady() const { return handle != VK_NULL_HANDLE; }

private:
	VkDevice device;
	VkShaderStageFlagBits stage;
	uint32_t codeSize;
	const uint32_t* code;
	VkShaderModule handle;
	VkPipelineShaderStageCreateInfo pipelineShaderStageInfo;
};

}

#endif