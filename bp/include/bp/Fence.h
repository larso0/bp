#ifndef BP_FENCE_H
#define BP_FENCE_H

#include <vulkan/vulkan.h>

namespace bp
{

class Fence
{
public:
	Fence() :
		device{VK_NULL_HANDLE},
		handle{VK_NULL_HANDLE} {}
	explicit Fence(VkDevice device) :
	Fence{}
	{
		init(device);
	}
	~Fence();

	void init(VkDevice device);
	void reset();
	bool wait(uint64_t timeout = UINT64_MAX);

	operator VkFence() { return handle; }

	VkFence getHandle() { return handle; }
	bool isReady() const { return handle != VK_NULL_HANDLE; }

private:
	VkDevice device;
	VkFence handle;
};

}

#endif