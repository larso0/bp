#ifndef BP_COMMANDPOOL_H
#define BP_COMMANDPOOL_H

#include "Queue.h"
#include <vector>
#include <utility>

namespace bp
{

class CommandPool
{
public:
	CommandPool() :
		queue{nullptr},
		handle{VK_NULL_HANDLE} {}
	CommandPool(
		Queue& queue,
		VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
	) :
		CommandPool{}
	{
		init(queue, flags);
	}
	~CommandPool();

	void init(Queue& queue,
		  VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	VkCommandBuffer allocateCommandBuffer(
		VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	std::vector<VkCommandBuffer> allocateCommandBuffers(
		uint32_t count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	void freeCommandBuffer(VkCommandBuffer cmdBuffer);
	void freeCommandBuffers(std::vector<VkCommandBuffer>& cmdBuffers);
	void submit(
		const std::vector<std::pair<VkSemaphore, VkPipelineStageFlags>>& waitSemaphores,
		const std::vector<VkCommandBuffer>& cmdBuffers,
		const std::vector<VkSemaphore>& signalSemaphores, VkFence fence = VK_NULL_HANDLE);
	void waitQueueIdle();

	operator VkCommandPool() { return handle; }

	Queue* getQueue() { return queue; }
	VkCommandPool getHandle() { return handle; }
	bool isReady() const { return handle != VK_NULL_HANDLE; }

private:
	Queue* queue;
	VkCommandPool handle;
};

}

#endif