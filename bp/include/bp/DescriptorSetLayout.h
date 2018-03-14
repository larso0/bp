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
		createFlags{0},
		handle{VK_NULL_HANDLE} {}
	DescriptorSetLayout(Device& device,
			    std::initializer_list<VkDescriptorSetLayoutBinding> bindings,
			    VkDescriptorSetLayoutCreateFlags createFlags = 0) :
		DescriptorSetLayout()
	{
		this->bindings = std::vector<VkDescriptorSetLayoutBinding>(bindings);
		init(device, createFlags);
	}
	~DescriptorSetLayout();

	void addLayoutBinding(const VkDescriptorSetLayoutBinding& binding);
	void init(Device& device, VkDescriptorSetLayoutCreateFlags createFlags = 0);

	operator VkDescriptorSetLayout() { return handle; }

	VkDescriptorSetLayout getHandle() { return handle; }

	const std::vector<VkDescriptorSetLayoutBinding>& getBindings() const { return bindings; }

	bool isPushDescriptorsEnabled() const
	{
		return static_cast<bool>(createFlags &
					 VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
	}
	bool isReady() const { return handle != VK_NULL_HANDLE; }

private:
	Device* device;
	VkDescriptorSetLayoutCreateFlags createFlags;
	VkDescriptorSetLayout handle;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
};

}

#endif