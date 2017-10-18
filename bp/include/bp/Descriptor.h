#ifndef BP_DESCRIPTOR_H
#define BP_DESCRIPTOR_H

#include <vulkan/vulkan.h>
#include <vector>

namespace bp
{

class Descriptor
{
public:
	Descriptor(VkDescriptorType type, uint32_t binding, uint32_t firstIndex) :
		type{type},
		binding{binding},
		firstIndex{firstIndex} {}
	virtual ~Descriptor() = default;

	VkDescriptorType getType() const { return type; }
	uint32_t getBinding() const { return binding; }
	uint32_t getFirstIndex() const { return firstIndex; }

private:
	VkDescriptorType type;
	uint32_t binding;
	uint32_t firstIndex;
};

}

#endif