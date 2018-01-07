#include <bp/Fence.h>
#include <stdexcept>

using namespace std;

namespace bp
{

Fence::~Fence()
{
	if (isReady()) vkDestroyFence(device, handle, nullptr);
}

void Fence::init(VkDevice device)
{
	VkFenceCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	VkResult result = vkCreateFence(device, &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create fence.");

	Fence::device = device;
}

void Fence::reset()
{
	vkResetFences(device, 1, &handle);
}

bool Fence::wait(uint64_t timeout)
{
	VkResult result = vkWaitForFences(device, 1, &handle, VK_TRUE, timeout);
	return result == VK_TIMEOUT;
}

}