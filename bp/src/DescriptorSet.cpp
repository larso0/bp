#include <bp/DescriptorSet.h>
#include <stdexcept>

using namespace std;

namespace bp
{

DescriptorSet::DescriptorSet(VkDevice device, VkDescriptorPool pool, DescriptorSetLayout& setLayout)
	:
	device{device},
	pool{pool},
	setLayout{setLayout}
{
	VkDescriptorSetLayout layoutHandle = setLayout;

	VkDescriptorSetAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	info.descriptorPool = pool;
	info.descriptorSetCount = 1;
	info.pSetLayouts = &layoutHandle;

	VkResult result = vkAllocateDescriptorSets(device, &info, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to allocate descriptor set.");
}

DescriptorSet::~DescriptorSet()
{
	vkFreeDescriptorSets(device, pool, 1, &handle);
}

}