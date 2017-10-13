#ifndef BP_CONTEXT_H
#define BP_CONTEXT_H

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include "Event.h"

namespace bp
{

/*
 * Context for a vulkan application.
 * Contains the vulkan instance, available physical devices, and optional debug callback.
 */
class Context
{
public:
	Context(bool enableDebug, const std::vector<const char*>& enabledExtensions,
		const VkApplicationInfo* applicationInfo = nullptr);
	~Context();

	operator VkInstance() { return instance; }

	VkInstance getInstance() { return instance; }
	std::vector<VkPhysicalDevice>& getPhysicalDevices() { return physicalDevices; }
	const std::vector<VkPhysicalDevice>& getPhysicalDevices() const { return physicalDevices; }

	/*
	 * Events for information, warning and error messages.
	 * Connect delegates to these events to recieve messages.
	 * Call these events to send messages to the delegates (operator()).
	 */
	Event<const std::string&> info;
	Event<const std::string&> warning;
	Event<const std::string&> error;
private:
	VkInstance instance;
	std::vector<VkPhysicalDevice> physicalDevices;
	VkDebugReportCallbackEXT debugReportCallback;
};

}

#endif