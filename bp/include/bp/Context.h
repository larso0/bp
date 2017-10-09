#ifndef BP_CONTEXT_H
#define BP_CONTEXT_H

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include "Event.h"
#include "DeviceQuery.h"

namespace bp
{

/*
 * Context for a vulkan application.
 * Contains the vulkan instance, available physical devices, and optional debug callback.
 */
class Context
{
public:
	Context() :
		instance(VK_NULL_HANDLE),
		debugReportCallback(VK_NULL_HANDLE),
		applicationInfo(nullptr),
		debugEnabled(false) {}
	Context(Context&& other);
	~Context();
	Context& operator=(Context&& other);

	/*
	 * Initialize the context.
	 * Must be called before the context is usable.
	 */
	void init();

	/*
	 * Create a device query that queries physical devices available to this context.
	 */
	DeviceQuery createDeviceQuery();

	/*
	 * Set application info, add extensions, and set debug mode before calling init.
	 */
	void setApplicationInfo(const VkApplicationInfo* info);
	void addExtension(const char* ext);
	void setDebugEnabled(bool enabled);

	VkInstance getInstance() { return instance; }
	std::vector<VkPhysicalDevice>& getPhysicalDevices() { return physicalDevices; }
	const VkApplicationInfo* getApplicationInfo() const { return applicationInfo; }
	const std::vector<const char*>& getEnabledExtensions() const { return enabledExtensions; }
	bool isDebugEnabled() const { return debugEnabled; }
	bool isReady() const { return instance != VK_NULL_HANDLE; }

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

	const VkApplicationInfo* applicationInfo;
	std::vector<const char*> enabledExtensions;
	bool debugEnabled;
};

}

#endif