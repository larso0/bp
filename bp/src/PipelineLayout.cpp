#include <bp/PipelineLayout.h>
#include <stdexcept>

using namespace std;

namespace bp
{

PipelineLayout::~PipelineLayout()
{
	if (handle != VK_NULL_HANDLE) vkDestroyPipelineLayout(device->getLogicalHandle(), handle,
							      nullptr);
}

void PipelineLayout::init(NotNull<Device> device)
{
	this->device = device;
	VkPipelineLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	info.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
	info.pSetLayouts = setLayouts.data();
	info.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
	info.pPushConstantRanges = pushConstantRanges.data();

	VkResult result = vkCreatePipelineLayout(*device, &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create pipeline layout.");
}

}