#ifndef BP_QUEUE_H
#define BP_QUEUE_H

#include <vulkan/vulkan.h>
#include <vector>

namespace bp
{

class Queue
{
public:
	Queue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex);

	void submit(
		const std::vector<std::pair<VkSemaphore, VkPipelineStageFlags>>& waitSemaphores,
		const std::vector<VkCommandBuffer>& cmdBuffers,
		const std::vector<VkSemaphore>& signalSemaphores, VkFence fence = VK_NULL_HANDLE);
	void waitIdle();

	operator VkQueue() { return handle; }

	VkDevice getDevice() { return device; }
	uint32_t getQueueFamilyIndex() const { return queueFamilyIndex; }
	uint32_t getQueueIndex() const { return queueIndex; }
	VkQueue getHandle() { return handle; }
private:
	VkDevice device;
	uint32_t queueFamilyIndex;
	uint32_t queueIndex;
	VkQueue handle;
};

}

#endif