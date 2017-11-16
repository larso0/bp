#ifndef BP_SHADER_H
#define BP_SHADER_H

#include "Device.h"
#include "Pointer.h"
#include <string>

namespace bp
{

class Shader
{
public:
	Shader() :
		device{nullptr},
		stage{VK_SHADER_STAGE_ALL},
		handle{VK_NULL_HANDLE},
		pipelineShaderStageInfo{} {}
	Shader(NotNull<Device> device, VkShaderStageFlagBits stage, uint32_t codeSize,
	       const uint32_t* code) :
		Shader{}
	{
		init(device, stage, codeSize, code);
	}
	Shader(NotNull<Device> device, VkShaderStageFlagBits stage, const std::string& glslSource) :
		Shader{}
	{
		init(device, stage, glslSource);
	}
	~Shader();

	void init(NotNull<Device> device, VkShaderStageFlagBits stage, uint32_t codeSize,
		  const uint32_t* code);
	void init(NotNull<Device> device, VkShaderStageFlagBits stage,
		  const std::string& glslSource);

	operator VkShaderModule() { return handle; }

	VkShaderModule getHandle() { return handle; }
	VkShaderStageFlags getStage() const { return stage; }
	const VkPipelineShaderStageCreateInfo& getPipelineShaderStageInfo() const
	{
		return pipelineShaderStageInfo;
	}
	bool isReady() const { return handle != VK_NULL_HANDLE; }

private:
	Device* device;
	VkShaderStageFlagBits stage;
	VkShaderModule handle;
	VkPipelineShaderStageCreateInfo pipelineShaderStageInfo;
};

}

#endif