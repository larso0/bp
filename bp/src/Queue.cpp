#include <bp/Queue.h>
#include <stdexcept>

using namespace std;

namespace bp
{

Queue::Queue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex) :
	device{device},
	queueFamilyIndex{queueFamilyIndex},
	queueIndex{queueIndex}
{
	vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &handle);
}

void Queue::submit(const vector<pair<VkSemaphore, VkPipelineStageFlags>>& waitSemaphores,
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

	vkQueueSubmit(handle, 1, &submitInfo, fence);
}

void Queue::waitIdle()
{
	vkQueueWaitIdle(handle);
}

}