#ifndef BP_DEVICE_H
#define BP_DEVICE_H

#include "Context.h"
#include "Queue.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace bp
{

struct DeviceRequirements
{
	DeviceRequirements(VkQueueFlags queues, const VkPhysicalDeviceFeatures& features,
			   VkSurfaceKHR surface, const std::vector<const char*>& extensions) :
		queues{queues},
		features{features},
		surface{surface},
		extensions{extensions} {}
	DeviceRequirements() :
		queues{0},
		features{},
		surface{VK_NULL_HANDLE} {}

	VkQueueFlags queues;
	VkPhysicalDeviceFeatures features;
	VkSurfaceKHR surface;
	std::vector<const char*> extensions;
};

bool queryDevice(VkPhysicalDevice device, const DeviceRequirements& requirements);
std::vector<VkPhysicalDevice> queryDevices(const Context& context,
					   const DeviceRequirements& requirements);

class Device
{
public:
	Device(const Context& context, const DeviceRequirements& requirements);
	Device(VkPhysicalDevice physicalDevice, const DeviceRequirements& requirements);
	~Device();

	operator VkPhysicalDevice() { return physical; }
	operator VkDevice() { return logical; }

	VkPhysicalDevice getPhysicalHandle() { return physical; }
	VkDevice getLogicalHandle() { return logical; }
	uint32_t getQueueCount() const { return static_cast<uint32_t>(queues.size()); }
	Queue& getQueue(uint32_t index = 0);
	Queue& getGraphicsQueue();
	Queue& getComputeQueue();
	Queue& getTransferQueue();
	Queue& getSparseBindingQueue();

private:
	VkPhysicalDevice physical;
	VkDevice logical;

	struct QueueInfo
	{
		uint32_t familyIndex;
		VkQueueFlags flags;
	};
	std::vector<QueueInfo> queueInfos;
	std::vector<Queue> queues;

	void createLogicalDevice(const DeviceRequirements& requirements);
	void createQueues();

	std::vector<VkDeviceQueueCreateInfo>
	setupQueueCreateInfos(const DeviceRequirements& requirements);
};

}

#endif