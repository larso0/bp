#include <bp/Semaphore.h>
#include <stdexcept>

using namespace std;

namespace bp
{

Semaphore::~Semaphore()
{
	if (isReady()) vkDestroySemaphore(device, handle, nullptr);
}

void Semaphore::init(VkDevice device)
{
	VkSemaphoreCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkResult result = vkCreateSemaphore(device, &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create semaphore.");

	Semaphore::device = device;
}

}