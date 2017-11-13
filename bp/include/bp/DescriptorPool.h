#ifndef BP_DESCRIPTORPOOL_H
#define BP_DESCRIPTORPOOL_H

#include "Device.h"
#include "Pointer.h"
#include <vector>

namespace bp
{

class DescriptorPool
{
public:
	DescriptorPool() :
		device{VK_NULL_HANDLE},
		handle{VK_NULL_HANDLE} {}
	DescriptorPool(NotNull<Device> device, const std::vector<VkDescriptorPoolSize>& poolSizes,
		       uint32_t maxSets) :
		DescriptorPool()
	{
		init(device, poolSizes, maxSets);
	}
	~DescriptorPool();

	void init(NotNull<Device> device, const std::vector<VkDescriptorPoolSize>& poolSizes,
		  uint32_t maxSets);

	operator VkDescriptorPool() { return handle; }

	VkDescriptorPool getHandle() { return handle; }

	bool isReady() const { return handle != VK_NULL_HANDLE; }

private:
	Device* device;
	VkDescriptorPool handle;
};

}

#endif