#include <bp/Shader.h>
#include <stdexcept>

using namespace std;

namespace bp
{

void Shader::init(NotNull<Device> device, VkShaderStageFlagBits stage, uint32_t codeSize,
		  const uint32_t* code)
{
	if (isReady()) throw runtime_error("Shader already initialized.");
	this->device = device;
	this->stage = stage;

	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.codeSize = codeSize;
	info.pCode = code;

	VkResult result = vkCreateShaderModule(*device, &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create shader module.");

	pipelineShaderStageInfo.sType =
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	pipelineShaderStageInfo.stage = stage;
	pipelineShaderStageInfo.module = handle;
	pipelineShaderStageInfo.pName = "main";
	pipelineShaderStageInfo.pSpecializationInfo = nullptr;
}

Shader::~Shader()
{
	vkDestroyShaderModule(*device, handle, nullptr);
}

}