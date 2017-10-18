#ifndef BP_DESCRIPTORSET_H
#define BP_DESCRIPTORSET_H

#include <vulkan/vulkan.h>
#include <vector>
#include "DescriptorSetLayout.h"
#include "Descriptor.h"

namespace bp
{

class DescriptorSet
{
public:
	DescriptorSet(VkDevice device, VkDescriptorPool pool, DescriptorSetLayout& setLayout);
	~DescriptorSet();

	void bind(const Descriptor* descriptor);
	void update();

	operator VkDescriptorSet() { return handle; }

	VkDescriptorSet getHandle() { return handle; }

private:
	VkDevice device;
	VkDescriptorPool pool;
	const DescriptorSetLayout& setLayout;
	VkDescriptorSet handle;
	std::vector<VkWriteDescriptorSet> descriptorWrites;
};

}

#endif