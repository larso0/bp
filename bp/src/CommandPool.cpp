#include <bp/CommandPool.h>
#include <stdexcept>

using namespace std;

namespace bp
{

CommandPool::~CommandPool()
{
	if (isReady())
		vkDestroyCommandPool(queue->getDevice(), handle, nullptr);
}

void CommandPool::init(NotNull<Queue> queue, VkCommandPoolCreateFlags flags)
{
	VkCommandPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.flags = flags;
	info.queueFamilyIndex = queue->getQueueFamilyIndex();

	VkResult result = vkCreateCommandPool(queue->getDevice(), &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create command pool.");
	CommandPool::queue = queue;
}

VkCommandBuffer CommandPool::allocateCommandBuffer(VkCommandBufferLevel level)
{
	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandPool = handle;
	info.level = level;
	info.commandBufferCount = 1;

	VkCommandBuffer cmdBuffer;
	VkResult result = vkAllocateCommandBuffers(queue->getDevice(), &info, &cmdBuffer);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to allocate command buffer.");

	return cmdBuffer;
}

vector<VkCommandBuffer> CommandPool::allocateCommandBuffers(uint32_t count,
								 VkCommandBufferLevel level)
{
	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.commandPool = handle;
	info.level = level;
	info.commandBufferCount = count;

	vector<VkCommandBuffer> cmdBuffers(count);
	VkResult result = vkAllocateCommandBuffers(queue->getDevice(), &info, cmdBuffers.data());
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to allocate command buffers.");

	return cmdBuffers;
}

void CommandPool::freeCommandBuffer(VkCommandBuffer cmdBuffer)
{
	vkFreeCommandBuffers(queue->getDevice(), handle, 1, &cmdBuffer);
}

void CommandPool::freeCommandBuffers(vector<VkCommandBuffer>& cmdBuffers)
{
	vkFreeCommandBuffers(queue->getDevice(), handle, static_cast<uint32_t>(cmdBuffers.size()),
			     cmdBuffers.data());
}

void CommandPool::submit(const vector<pair<VkSemaphore, VkPipelineStageFlags>>& waitSemaphores,
			  const vector<VkCommandBuffer>& cmdBuffers,
			  const vector<VkSemaphore>& signalSemaphores, VkFence fence)
{
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());

	vector<VkSemaphore> waitSems;
	waitSems.reserve(waitSemaphores.size());
	vector<VkPipelineStageFlags> waitStages;
	waitStages.reserve(waitSemaphores.size());
	for (auto& w : waitSemaphores)
	{
		waitSems.push_back(w.first);
		waitStages.push_back(w.second);
	}

	submitInfo.pWaitSemaphores = waitSems.data();
	submitInfo.pWaitDstStageMask = waitStages.data();
	submitInfo.commandBufferCount = static_cast<uint32_t>(cmdBuffers.size());
	submitInfo.pCommandBuffers = cmdBuffers.data();
	submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
	submitInfo.pSignalSemaphores = signalSemaphores.data();

	vkQueueSubmit(*queue, 1, &submitInfo, fence);
}

void CommandPool::waitQueueIdle()
{
	vkQueueWaitIdle(*queue);
}

}