#ifndef BP_DESCRIPTORSET_H
#define BP_DESCRIPTORSET_H

#include <vulkan/vulkan.h>
#include "DescriptorSetLayout.h"

namespace bp
{

class DescriptorSet
{
public:
	DescriptorSet(VkDevice device, VkDescriptorPool pool, DescriptorSetLayout& setLayout);
	~DescriptorSet();

	operator VkDescriptorSet() { return handle; }

	VkDescriptorSet getHandle() { return handle; }

private:
	VkDevice device;
	VkDescriptorPool pool;
	const DescriptorSetLayout& setLayout;
	VkDescriptorSet handle;
};

}

#endif