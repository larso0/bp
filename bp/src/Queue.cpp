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

}