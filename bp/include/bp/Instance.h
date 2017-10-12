#ifndef BP_CONTEXT_H
#define BP_CONTEXT_H

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include "Event.h"

namespace bp
{

/*
 * Instance for a vulkan application.
 * Contains the vulkan instance, available physical devices, and optional debug callback.
 */
class Instance
{
public:
	Instance(bool enableDebug, const std::vector<std::string>& enabledExtensions,
		const VkApplicationInfo* applicationInfo = nullptr);
	~Instance();

	operator VkInstance() { return instance; }

	VkInstance getInstance() { return instance; }
	std::vector<VkPhysicalDevice>& getPhysicalDevices() { return physicalDevices; }
	const std::vector<VkPhysicalDevice>& getPhysicalDevices() const { return physicalDevices; }

	/*
	 * Events for information, warning and error messages.
	 * Connect delegates to these events to recieve messages.
	 * Call these events to send messages to the delegates (operator()).
	 */
	Event<const std::string&> infoEvent;
	Event<const std::string&> warningEvent;
	Event<const std::string&> errorEvent;
private:
	VkInstance instance;
	std::vector<VkPhysicalDevice> physicalDevices;
	VkDebugReportCallbackEXT debugReportCallback;
};

}

#endif