#ifndef DEVICEQUERY_H
#define DEVICEQUERY_H

#include <vulkan/vulkan.h>
#include <vector>

namespace bp
{

/*
 * Query for physical devices and filter by your needs.
 * Use the physical device(s) from the results to create logical device(s).
 */
class DeviceQuery
{
public:
	DeviceQuery() :
		physicalDeviceCount(0),
		physicalDevices(nullptr),
		queues(0),
		features(nullptr),
		surface(VK_NULL_HANDLE) {}

	/*
	 * Set the list of physical devices of which to query.
	 */
	void setPhysicalDevices(uint32_t count, VkPhysicalDevice* devices);

	/*
	 * Filter the results by setting required queues, features, surface to support (graphics
	 * queue must also be supported), and required extensions.
	 */
	void setQueues(VkQueueFlags queues);
	void setFeatures(VkPhysicalDeviceFeatures* features);
	void setSurface(VkSurfaceKHR surface);
	void addExtension(const char* ext);

	/*
	 * Get the resulting list of suitable physical devices.
	 */
	std::vector<VkPhysicalDevice>& getResults();
private:
	uint32_t physicalDeviceCount;
	VkPhysicalDevice* physicalDevices;

	VkQueueFlags queues;
	VkPhysicalDeviceFeatures* features;
	VkSurfaceKHR surface;
	std::vector<const char*> extensions;

	std::vector<VkPhysicalDevice> results;

	bool queryDevice(VkPhysicalDevice device);
};

}

#endif