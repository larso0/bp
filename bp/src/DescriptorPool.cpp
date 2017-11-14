#include <bp/DescriptorPool.h>
#include <stdexcept>

using namespace std;

namespace bp
{

void DescriptorPool::init(NotNull<Device> device, const std::vector<VkDescriptorPoolSize>& poolSizes,
			  uint32_t maxSets)
{
	if (isReady()) throw runtime_error("Descriptor pool is already initialized.");
	this->device = device;
	VkDescriptorPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	info.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	info.pPoolSizes = poolSizes.data();
	info.maxSets = maxSets;

	VkResult result = vkCreateDescriptorPool(*device, &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create descriptor pool.");
}

DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(*device, handle, nullptr);
}

}