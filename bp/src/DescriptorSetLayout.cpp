#include <bp/DescriptorSetLayout.h>
#include <stdexcept>

using namespace std;

namespace bp
{

DescriptorSetLayout::DescriptorSetLayout(VkDevice device,
					 const std::vector<VkDescriptorSetLayoutBinding>& bindings)
	:
	device{device},
	bindings{bindings}
{
	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = static_cast<uint32_t>(bindings.size());
	info.pBindings = bindings.data();

	VkResult result = vkCreateDescriptorSetLayout(device, &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create descriptor set layout.");
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(device, handle, nullptr);
}

}