#ifndef BP_DESCRIPTORSETLAYOUT_H
#define BP_DESCRIPTORSETLAYOUT_H

#include <vulkan/vulkan.h>
#include <vector>

namespace bp
{

class DescriptorSetLayout
{
public:
	DescriptorSetLayout(VkDevice device,
			    const std::vector<VkDescriptorSetLayoutBinding>& bindings);
	~DescriptorSetLayout();

	operator VkDescriptorSetLayout() { return handle; }

	VkDescriptorSetLayout getHandle() { return handle; }
	const std::vector<VkDescriptorSetLayoutBinding>& getBindings() const { return bindings; }

private:
	VkDevice device;
	VkDescriptorSetLayout handle;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
};

}

#endif