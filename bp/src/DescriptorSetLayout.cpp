#include <bp/DescriptorSetLayout.h>
#include <stdexcept>

using namespace std;

namespace bp
{

void DescriptorSetLayout::addLayoutBinding(const VkDescriptorSetLayoutBinding& binding)
{
	if (isReady()) throw runtime_error("Descriptor set layout already initialized.");
	bindings.push_back(binding);
}

void DescriptorSetLayout::init(NotNull<Device> device)
{
	if (isReady()) throw runtime_error("Descriptor set layout already initialized.");
	this->device = device;

	VkDescriptorSetLayoutCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	info.bindingCount = static_cast<uint32_t>(bindings.size());
	info.pBindings = bindings.data();

	VkResult result = vkCreateDescriptorSetLayout(*device, &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create descriptor set layout.");
}

DescriptorSetLayout::~DescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(*device, handle, nullptr);
}

}