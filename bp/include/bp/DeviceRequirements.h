#ifndef BP_DEVICEREQUIREMENTS_H
#define BP_DEVICEREQUIREMENTS_H

#include <vulkan/vulkan.h>
#include "Instance.h"
#include <vector>
#include <string>

namespace bp
{

/*
 * Require that there exists a queue family that support all the flags given in
 * QueueRequirements::flags, and that it is possible to create QueueRequirements::count
 * such queues. If surface is not VK_NULL_HANDLE, the given surface must be supported for
 * presenting to. Priorities are only needed when creating a device.
 */
struct QueueRequirements
{
	VkQueueFlags flags;
	uint32_t count;
	VkSurfaceKHR surface;
	std::vector<float> priorities;
};

/*
 * Device requirements used to check if physical devices have the desired functionality.
 */
class DeviceRequirements
{
public:
	/*
	 * Require that there exists queue families to support all queue flags.
	 * The capabilities can be spread among multiple queue families. It is not required that a
	 * single queue family must support all flags, if there exist other queue families that
	 * support the missing flags.
	 */
	VkQueueFlags queueCapabilities;

	/*
	 * Required features.
	 */
	VkPhysicalDeviceFeatures features;

	/*
	 * A surface that is required to be supported for presenting.
	 */
	VkSurfaceKHR surface;

	std::vector<QueueRequirements> queues;
	std::vector<std::string> extensions;
	std::vector<std::string> layers;

	DeviceRequirements() :
		queueCapabilities{0},
		features{},
		surface{VK_NULL_HANDLE} {}

	/*
	 * Add a required queue.
	 */
	void addQueue(const QueueRequirements& queue) { queues.push_back(queue); }
	template <typename Iterator>
	void addQueues(Iterator begin, Iterator end)
	{
		queues.insert(queues.end(), begin, end);
	}

	/*
	 * Add device extensions that should be supported by the device.
	 */
	void addExtension(const std::string& extensionName) { extensions.push_back(extensionName); }
	template <typename Iterator>
	void addExtensions(Iterator begin, Iterator end)
	{
		extensions.insert(extensions.end(), begin, end);
	}

	/*
	 * Add device layers that should be supported by the device.
	 */
	void addLayer(const std::string& layerName) { extensions.push_back(layerName); }
	template <typename Iterator>
	void addLayers(Iterator begin, Iterator end)
	{
		layers.insert(layers.end(), begin, end);
	}

	bool meetsQueueCapabilities(VkQueueFlags flags) const
	{
		return (flags & queueCapabilities) == queueCapabilities;
	}
};

/*
 * Query a single physical device.
 * Returns true if it meets the requirements, false otherwise.
 */
bool queryDevice(VkPhysicalDevice device, const DeviceRequirements& requirements);

/*
 * Query a list of physical devices and return the devices that meet the requirements.
 */
std::vector<VkPhysicalDevice> queryDevices(const std::vector<VkPhysicalDevice>& devices,
					   const DeviceRequirements& requirements);

/*
 * Query a list of physical devices from an Instance and return the devices that meet the
 * requirements.
 */
std::vector<VkPhysicalDevice> queryDevices(const Instance& instance,
					   const DeviceRequirements& requirements);

}

#endif