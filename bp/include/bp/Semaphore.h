#ifndef BP_SEMAPHORE_H
#define BP_SEMAPHORE_H

#include <vulkan/vulkan.h>

namespace bp
{

class Semaphore
{
public:
	Semaphore() :
		device{VK_NULL_HANDLE},
		handle{VK_NULL_HANDLE} {}
	explicit Semaphore(VkDevice device) :
		Semaphore{}
	{
		init(device);
	}
	~Semaphore();

	void init(VkDevice device);

	operator VkSemaphore() { return handle; }

	VkSemaphore getHandle() { return handle; }
	bool isReady() const { return handle != VK_NULL_HANDLE; }

private:
	VkDevice device;
	VkSemaphore handle;
};

}

#endif