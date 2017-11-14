#ifndef BP_CONTEXT_H
#define BP_CONTEXT_H

#include <vulkan/vulkan.h>
#include <string>
#include <initializer_list>
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
	Instance() :
		handle{VK_NULL_HANDLE},
		debugReportCallback{VK_NULL_HANDLE} {}
	Instance(bool enableDebug, std::initializer_list<std::string> enabledExtensions,
		const VkApplicationInfo* applicationInfo = nullptr) :
		Instance{}
	{
		init(enableDebug, enabledExtensions, applicationInfo);
	}
	~Instance();

	template <typename Iterator>
	void enableExtensions(Iterator begin, Iterator end)
	{
		enabledExtensions.insert(enabledExtensions.end(), begin, end);
	}
	void enableExtension(const std::string& extensionName);
	void init(bool enableDebug, std::initializer_list<std::string> enabledExtensions,
		  const VkApplicationInfo* applicationInfo = nullptr);
	void init(bool enableDebug, const VkApplicationInfo* applicationInfo = nullptr);

	operator VkInstance() { return handle; }

	VkInstance getHandle() { return handle; }
	std::vector<VkPhysicalDevice>& getPhysicalDevices() { return physicalDevices; }
	const std::vector<VkPhysicalDevice>& getPhysicalDevices() const { return physicalDevices; }
	const std::vector<std::string>& getEnabledExtensions() const { return enabledExtensions; }
	bool isReady() const { return handle != VK_NULL_HANDLE; }

	/*
	 * Events for information, warning and error messages.
	 * Connect delegates to these events to recieve messages.
	 * Call these events to send messages to the delegates (operator()).
	 */
	Event<const std::string&> infoEvent;
	Event<const std::string&> warningEvent;
	Event<const std::string&> errorEvent;
private:
	VkInstance handle;
	std::vector<VkPhysicalDevice> physicalDevices;
	VkDebugReportCallbackEXT debugReportCallback;

	std::vector<std::string> enabledExtensions;
};

}

#endif