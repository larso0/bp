#include <bp/Shader.h>
#include <stdexcept>

using namespace std;

namespace bp
{

Shader::~Shader()
{
	if (isReady())
		vkDestroyShaderModule(device, handle, nullptr);
}

void Shader::init()
{
	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = codeSize;
	info.pCode = code;

	VkResult result = vkCreateShaderModule(device, &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create shader module.");

	pipelineShaderStageInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineShaderStageInfo.stage = stage;
	pipelineShaderStageInfo.module = handle;
	pipelineShaderStageInfo.pName = "main";
	pipelineShaderStageInfo.pSpecializationInfo = nullptr;
}

void Shader::setDevice(VkDevice device)
{
	if (isReady())
		throw runtime_error("Failed to alter device, shader module already created.");
	this->device = device;
}

void Shader::setStage(VkShaderStageFlagBits stage)
{
	if (isReady())
		throw runtime_error("Failed to alter stage, shader module already created.");
	this->stage = stage;
}

void Shader::setCode(uint32_t size, const uint32_t* code)
{
	if (isReady())
		throw runtime_error("Failed to alter code, shader module already created.");
	codeSize = size;
	this->code = code;
}

}