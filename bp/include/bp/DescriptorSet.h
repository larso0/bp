#ifndef BP_DESCRIPTORSET_H
#define BP_DESCRIPTORSET_H

#include "Device.h"
#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"
#include "Descriptor.h"
#include <vector>

namespace bp
{

class DescriptorSet
{
public:
	DescriptorSet() :
		device{nullptr},
		pool{nullptr},
		handle{VK_NULL_HANDLE} {}
	DescriptorSet(Device& device, DescriptorPool& pool, DescriptorSetLayout& layout) :
		DescriptorSet()
	{
		init(device, pool, layout);
	}
	~DescriptorSet();

	void init(Device& device, DescriptorPool& pool,
		  DescriptorSetLayout& layout);
	void bind(const Descriptor& descriptor);
	void update();

	operator VkDescriptorSet() { return handle; }

	VkDescriptorSet getHandle() { return handle; }

	bool isReady() const { return handle != VK_NULL_HANDLE; }

private:
	Device* device;
	DescriptorPool* pool;
	VkDescriptorSet handle;
	std::vector<const Descriptor*> descriptors;
};

}

#endif