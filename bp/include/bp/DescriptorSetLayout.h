#ifndef BP_DESCRIPTORSETLAYOUT_H
#define BP_DESCRIPTORSETLAYOUT_H

#include "Device.h"
#include <initializer_list>
#include <vector>

namespace bp
{

class DescriptorSetLayout
{
public:
	DescriptorSetLayout() :
		device{nullptr},
		handle{VK_NULL_HANDLE} {}
	DescriptorSetLayout(Device& device,
			    std::initializer_list<VkDescriptorSetLayoutBinding> bindings) :
		DescriptorSetLayout()
	{
		this->bindings = std::vector<VkDescriptorSetLayoutBinding>(bindings);
		init(device);
	}
	~DescriptorSetLayout();

	void addLayoutBinding(const VkDescriptorSetLayoutBinding& binding);
	void init(Device& device);

	operator VkDescriptorSetLayout() { return handle; }

	VkDescriptorSetLayout getHandle() { return handle; }

	const std::vector<VkDescriptorSetLayoutBinding>& getBindings() const { return bindings; }

	bool isReady() const { return handle != VK_NULL_HANDLE; }

private:
	Device* device;
	VkDescriptorSetLayout handle;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
};

}

#endif