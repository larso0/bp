#include <bp/Shader.h>
#include <shaderc/shaderc.hpp>
#include <stdexcept>

using namespace std;

namespace bp
{

void Shader::init(Device& device, VkShaderStageFlagBits stage, uint32_t codeSize,
		  const uint32_t* code)
{
	if (isReady()) throw runtime_error("Shader already initialized.");
	Shader::device = &device;
	Shader::stage = stage;

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

void Shader::init(Device& device, VkShaderStageFlagBits stage,
		  const std::string& glslSource)
{
	shaderc::Compiler compiler;

	shaderc_shader_kind kind;
	switch (stage)
	{
	case VK_SHADER_STAGE_VERTEX_BIT:
		kind = shaderc_vertex_shader;
		break;
	case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		kind = shaderc_tess_control_shader;
		break;
	case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		kind = shaderc_tess_evaluation_shader;
		break;
	case VK_SHADER_STAGE_GEOMETRY_BIT:
		kind = shaderc_geometry_shader;
		break;
	case VK_SHADER_STAGE_FRAGMENT_BIT:
		kind = shaderc_fragment_shader;
		break;
	case VK_SHADER_STAGE_COMPUTE_BIT:
		kind = shaderc_compute_shader;
		break;
	default:
		kind = shaderc_glsl_infer_from_source;
		break;
	}

	auto result = compiler.CompileGlslToSpv(glslSource, kind, "bp Shader");

	if (result.GetNumErrors() > 0)
		throw runtime_error(result.GetErrorMessage());

	const uint32_t* code = result.begin();
	uint32_t size = static_cast<uint32_t>(reinterpret_cast<uint64_t>(result.end())
					      - reinterpret_cast<uint64_t>(code));
	init(device, stage, size, code);
}

Shader::~Shader()
{
	vkDestroyShaderModule(*device, handle, nullptr);
}

}