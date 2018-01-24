#ifndef BP_EVENT_H
#define BP_EVENT_H

#include <vulkan/vulkan.h>

namespace bp
{

class Event
{
public:
	Event() :
		device{VK_NULL_HANDLE},
		handle{VK_NULL_HANDLE} {}
	explicit Event(VkDevice device) :
		Event{}
	{
		init(device);
	}
	~Event();

	void init(VkDevice device);

	operator VkEvent() { return handle; }

	VkEvent getHandle() { return handle; }
	bool isReady() const { return handle != VK_NULL_HANDLE; }

private:
	VkDevice device;
	VkEvent handle;
};

}

#endif