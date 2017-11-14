#ifndef BP_DESCRIPTORSET_H
#define BP_DESCRIPTORSET_H

#include "Device.h"
#include "Pointer.h"
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
	DescriptorSet(NotNull<Device> device, NotNull<DescriptorPool> pool,
		      NotNull<DescriptorSetLayout> layout) :
		DescriptorSet()
	{
		init(device, pool, layout);
	}
	~DescriptorSet();

	void init(NotNull<Device> device, NotNull<DescriptorPool> pool,
		  NotNull<DescriptorSetLayout> layout);
	void bind(const NotNull<Descriptor> descriptor);
	void update();

	operator VkDescriptorSet() { return handle; }

	VkDescriptorSet getHandle() { return handle; }

	bool isReady() const { return handle != VK_NULL_HANDLE; }

private:
	Device* device;
	DescriptorPool* pool;
	VkDescriptorSet handle;
	std::vector<VkWriteDescriptorSet> descriptorWrites;
};

}

#endif