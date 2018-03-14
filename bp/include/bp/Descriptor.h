#ifndef BP_DESCRIPTOR_H
#define BP_DESCRIPTOR_H

#include <vulkan/vulkan.h>
#include <vector>

namespace bp
{

class Descriptor
{
public:
	Descriptor() :
		type{VK_DESCRIPTOR_TYPE_MAX_ENUM},
		binding{0},
		firstIndex{0} {}
	Descriptor(VkDescriptorType type, uint32_t binding, uint32_t firstIndex) :
		type{type},
		binding{binding},
		firstIndex{firstIndex} {}
	virtual ~Descriptor() = default;

	VkDescriptorType getType() const
	{
		return type;
	}
	void setType(VkDescriptorType type)
	{
		Descriptor::type = type;
	}
	uint32_t getBinding() const
	{
		return binding;
	}
	void setBinding(uint32_t binding)
	{
		Descriptor::binding = binding;
	}
	uint32_t getFirstIndex() const
	{
		return firstIndex;
	}
	void setFirstIndex(uint32_t firstIndex)
	{
		Descriptor::firstIndex = firstIndex;
	}

	virtual VkWriteDescriptorSet getWriteInfo() const = 0;

private:
	VkDescriptorType type;
	uint32_t binding;
	uint32_t firstIndex;
};

}

#endif