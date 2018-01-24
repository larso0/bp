#include <bp/Event.h>
#include <stdexcept>

using std::runtime_error;

namespace bp
{

Event::~Event()
{
	if (isReady())
		vkDestroyEvent(device, handle, nullptr);
}

void Event::init(VkDevice device)
{
	VkEventCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;

	VkResult result = vkCreateEvent(device, &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create event.");

	Event::device = device;
}

}