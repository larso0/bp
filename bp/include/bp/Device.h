#ifndef BP_DEVICE_H
#define BP_DEVICE_H

#include "Context.h"
#include "Queue.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace bp
{

struct DeviceRequirements
{
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
	std::shared_ptr<Queue> getQueue(uint32_t index = 0);
	std::shared_ptr<Queue> getGraphicsQueue();
	std::shared_ptr<Queue> getComputeQueue();
	std::shared_ptr<Queue> getTransferQueue();
	std::shared_ptr<Queue> getSparseBindingQueue();

private:
	VkPhysicalDevice physical;
	VkDevice logical;

	struct QueueInfo
	{
		uint32_t familyIndex;
		VkQueueFlags flags;
	};
	std::vector<QueueInfo> queueInfos;
	std::vector<std::shared_ptr<Queue>> queues;

	void createLogicalDevice(const DeviceRequirements& requirements);
	void createQueues();

	std::vector<VkDeviceQueueCreateInfo>
	setupQueueCreateInfos(const DeviceRequirements& requirements);
};

}

#endif