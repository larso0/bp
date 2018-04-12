#ifndef BP_PIPELINE_H
#define BP_PIPELINE_H

#include "Device.h"
#include "Shader.h"
#include "RenderPass.h"
#include <vector>
#include <initializer_list>

namespace bp
{

class Pipeline
{
public:
	Pipeline() :
		device{nullptr},
		handle{VK_NULL_HANDLE},
		layout{VK_NULL_HANDLE} {}
	virtual ~Pipeline()
	{
		if (isReady())
			vkDestroyPipeline(*device, handle, nullptr);
	}

	void addShaderStageInfo(const VkPipelineShaderStageCreateInfo& info)
	{
		shaderStageInfos.push_back(info);
	}

	template <typename Iterator>
	void addShaderStageInfos(Iterator begin, Iterator end)
	{
		shaderStageInfos.insert(shaderStageInfos.end(), begin, end);
	}

	operator VkPipeline() { return handle; }

	bool isReady() const { return handle != VK_NULL_HANDLE; }

	VkPipeline getHandle() { return handle; }
	VkPipelineLayout getPipelineLayout() { return layout; }

protected:
	Device* device;
	VkPipeline handle;
	VkPipelineLayout layout;
	std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos;
};

}

#endif