#include <bp/Instance.h>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace bp
{

static const char* VALIDATION_LAYER = "VK_LAYER_LUNARG_standard_validation";

static VKAPI_ATTR VkBool32
VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags,
			 VkDebugReportObjectTypeEXT, uint64_t, size_t,
			 int32_t, const char* pLayerPrefix,
			 const char* pMessage, void* pUserData)
{
	stringstream ss;
	ss << pLayerPrefix << ": " << pMessage;
	Instance* instance = static_cast<Instance*>(pUserData);
	if (flags & (VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT))
		instance->infoEvent(ss.str());
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		instance->warningEvent(ss.str());
	else
		instance->errorEvent(ss.str());

	return VK_FALSE;
}

Instance::~Instance()
{
	if (debugReportCallback != VK_NULL_HANDLE)
	{
		PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback =
			reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
				vkGetInstanceProcAddr(handle, "vkDestroyDebugReportCallbackEXT"));
		vkDestroyDebugReportCallback(handle, debugReportCallback, nullptr);
	}
	vkDestroyInstance(handle, nullptr);
}

void Instance::enableExtension(const std::string& extensionName)
{
	if (isReady())
		throw runtime_error("Failed to enable extension, instance already initialized.");
	enabledExtensions.push_back(extensionName);
}

void Instance::init(bool enableDebug, std::initializer_list<std::string> enabledExtensions,
		    const VkApplicationInfo* applicationInfo)
{
	this->enabledExtensions.insert(this->enabledExtensions.end(), enabledExtensions.begin(),
				       enabledExtensions.end());
	init(enableDebug, applicationInfo);
}

void Instance::init(bool enableDebug, const VkApplicationInfo* applicationInfo)
{
	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = applicationInfo;

	vector<const char*> extensions;
	for (const string& ext : this->enabledExtensions)
		extensions.push_back(ext.c_str());

	if (enableDebug)
	{
		instanceInfo.enabledLayerCount = 1;
		instanceInfo.ppEnabledLayerNames = &VALIDATION_LAYER;
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceInfo.ppEnabledExtensionNames = extensions.data();

	VkResult result = vkCreateInstance(&instanceInfo, nullptr, &handle);

	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create instance.");

	uint32_t n = 0;
	vkEnumeratePhysicalDevices(handle, &n, nullptr);
	physicalDevices.resize(n);
	vkEnumeratePhysicalDevices(handle, &n, physicalDevices.data());

	if (enableDebug)
	{
		PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallback =
			reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
				vkGetInstanceProcAddr(handle, "vkCreateDebugReportCallbackEXT"));

		VkDebugReportCallbackCreateInfoEXT info = {};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
			     VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		info.pfnCallback = debugCallback;
		info.pUserData = static_cast<void*>(this);

		result = vkCreateDebugReportCallback(handle, &info, nullptr,
						     &debugReportCallback);

		if (result != VK_SUCCESS)
		{
			vkDestroyInstance(handle, nullptr);
			handle = VK_NULL_HANDLE;
			throw runtime_error("Failed to create debug report callback.");
		}
	}
}

}