#include <bp/Context.h>
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
	Context* context = static_cast<Context*>(pUserData);
	if (flags & (VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT))
		context->info(ss.str());
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		context->warning(ss.str());
	else
		context->error(ss.str());
}

Context::~Context()
{
	if (debugReportCallback != VK_NULL_HANDLE)
	{
		PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback =
			reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
				vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
		vkDestroyDebugReportCallback(instance, debugReportCallback, nullptr);
	}
	vkDestroyInstance(instance, nullptr);
}

Context::Context(bool enableDebug, const vector<string>& enabledExtensions,
		 const VkApplicationInfo* applicationInfo) :
	debugReportCallback{VK_NULL_HANDLE}
{
	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = applicationInfo;

	vector<const char*> extensions;
	for (const string& ext : enabledExtensions)
		extensions.push_back(ext.c_str());

	if (enableDebug)
	{
		instanceInfo.enabledLayerCount = 1;
		instanceInfo.ppEnabledLayerNames = &VALIDATION_LAYER;
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceInfo.ppEnabledExtensionNames = extensions.data();

	VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);

	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create Vulkan instance.");

	uint32_t n = 0;
	vkEnumeratePhysicalDevices(instance, &n, nullptr);
	physicalDevices.resize(n);
	vkEnumeratePhysicalDevices(instance, &n, physicalDevices.data());

	if (enableDebug)
	{
		PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallback =
			reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
				vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));

		VkDebugReportCallbackCreateInfoEXT info = {};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
			     VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		info.pfnCallback = debugCallback;
		info.pUserData = static_cast<void*>(this);

		result = vkCreateDebugReportCallback(instance, &info, nullptr,
						     &debugReportCallback);

		if (result != VK_SUCCESS)
		{
			vkDestroyInstance(instance, nullptr);
			throw runtime_error("Failed to create debug report callback.");
		}
	}
}

}