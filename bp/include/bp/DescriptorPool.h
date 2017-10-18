#ifndef BP_DESCRIPTORPOOL_H
#define BP_DESCRIPTORPOOL_H

#include <vulkan/vulkan.h>
#include <vector>

namespace bp
{

class DescriptorPool
{
public:
	DescriptorPool(VkDevice device, const std::vector<VkDescriptorPoolSize>& poolSizes,
		       uint32_t maxSets);
	~DescriptorPool();

	operator VkDescriptorPool() { return handle; }

	VkDescriptorPool getHandle() { return handle; }

private:
	VkDevice device;
	VkDescriptorPool handle;
};

}

#endif