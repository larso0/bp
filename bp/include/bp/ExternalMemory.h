#ifndef BP_EXTERNALMEMORY_H
#define BP_EXTERNALMEMORY_H

#include "Memory.h"

#include <array>
#include <string>

namespace bp
{

class ExternalMemory : public Memory
{
public:
	static const std::array<std::string, 2> requiredInstanceExtensions;
	static const std::array<std::string, 2> requiredDeviceExtensions;

	ExternalMemory(VkPhysicalDevice physicalDevice, VkDevice device,
		       const VkMemoryRequirements& requirements, void* opaque);
	~ExternalMemory();

	bool isMapped() const override { return true; }
	VkDeviceSize getSize() const override { return size; }
	void* getMapped() override { return opaque; }
	VkDeviceMemory getHandle() { return handle; }
	operator VkDeviceMemory() { return handle; }
private:
	VkDevice device;
	VkDeviceMemory handle;
	VkDeviceSize size;
	void* opaque;
};

}

#endif