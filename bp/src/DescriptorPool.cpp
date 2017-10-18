#include <bp/DescriptorPool.h>
#include <stdexcept>

using namespace std;

namespace bp
{

DescriptorPool::DescriptorPool(VkDevice device, const std::vector<VkDescriptorPoolSize>& poolSizes,
			       uint32_t maxSets) :
	device{device}
{
	VkDescriptorPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	info.pPoolSizes = poolSizes.data();
	info.maxSets = maxSets;

	VkResult result = vkCreateDescriptorPool(device, &info, nullptr, &handle);
}

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(device, handle, nullptr);
}

}