#include <bp/bp.h>
#include <bp/vk_includes.h>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <sstream>

using namespace std;

namespace bp {
	VkInstance instance = VK_NULL_HANDLE;
	vector<VkPhysicalDevice> physical_devices;

	static void glfw_error_callback(int error, const char* description) {
		cerr << "GLFW Error:" << description << endl;
	}

#ifndef NDEBUG
	static const char* validation_layer = "VK_LAYER_LUNARG_standard_validation";
	static VkDebugReportCallbackEXT dbg_callback;
	static PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallback = nullptr;
	static PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback = nullptr;

	static VKAPI_ATTR VkBool32 VKAPI_CALL
	dbg_report_callback(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t,
	                    const char* pLayerPrefix, const char* pMessage, void*) {
		cerr << pLayerPrefix << ": " << pMessage << endl;
		return VK_FALSE;
	}

	void load_dbg_ext() {
		vkCreateDebugReportCallback =
			(PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		vkDestroyDebugReportCallback =
			(PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	}

	void create_dbg_callback() {
		VkDebugReportCallbackCreateInfoEXT info;
		info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		info.flags
			= VK_DEBUG_REPORT_ERROR_BIT_EXT
			  | VK_DEBUG_REPORT_WARNING_BIT_EXT
			  | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
		info.pfnCallback = dbg_report_callback;
		info.pUserData = nullptr;

		VkResult result = vkCreateDebugReportCallback(instance, &info, nullptr, &dbg_callback);
		if (result != VK_SUCCESS) {
			throw runtime_error("Failed to create debug report callback.");
		}
	}

#endif

	static vector<const char*> get_required_ext_names() {
		unsigned n = 0;
		const char** es;
		es = glfwGetRequiredInstanceExtensions(&n);
		vector<const char*> ext_names(es, es + n);
#ifndef NDEBUG
		ext_names.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
		vkEnumerateInstanceExtensionProperties(nullptr, &n, nullptr);
		vector<VkExtensionProperties> available(n);
		vkEnumerateInstanceExtensionProperties(nullptr, &n, available.data());
		n = 0;
		for (const char* e : ext_names) {
			if (find_if(available.begin(), available.end(),
			            [e](const VkExtensionProperties& l) -> bool {
				            return strcmp(l.extensionName, e) == 0;
			            }) == available.end()) {
				stringstream ss;
				ss << "Required extension \"" << e << "\" is not available.";
				throw runtime_error(ss.str());
			}
		}
		return ext_names;
	}

	static vector<VkLayerProperties> get_layer_names() {
		uint32_t count = 0;
		vkEnumerateInstanceLayerProperties(&count, nullptr);
		vector<VkLayerProperties> layers(count);
		vkEnumerateInstanceLayerProperties(&count, layers.data());
		return layers;
	}

	void create_instance() {
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = nullptr;
		app_info.pApplicationName = "VulkanSandbox";
		app_info.applicationVersion = 1;
		app_info.pEngineName = "VulkanSandbox";
		app_info.engineVersion = 1;
		app_info.apiVersion = 0;

		VkInstanceCreateInfo instance_info = {};
		instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_info.pNext = nullptr;
		instance_info.flags = 0;
		instance_info.pApplicationInfo = &app_info;

		auto extensions = get_required_ext_names();
		instance_info.enabledExtensionCount = (uint32_t) extensions.size();
		instance_info.ppEnabledExtensionNames = extensions.data();

#ifdef NDEBUG
		instance_info.enabledLayerCount = 0;
		instance_info.ppEnabledLayerNames = nullptr;
#else
		{
			auto available = get_layer_names();
			auto pred = [](const VkLayerProperties& l) -> bool {
				return strcmp(l.layerName, validation_layer) == 0;
			};
			auto result = find_if(available.begin(), available.end(), pred);
			if (result == available.end()) {
				throw runtime_error("Validation layer is not available.");
			}
		}
		instance_info.enabledLayerCount = 1;
		instance_info.ppEnabledLayerNames = &validation_layer;
#endif

		VkResult result = vkCreateInstance(&instance_info, nullptr, &instance);
		if (result != VK_SUCCESS) {
			throw runtime_error("Failed to create Vulkan instance.");
		}
	}

	static void quit() {
#ifndef NDEBUG
		vkDestroyDebugReportCallback(instance, dbg_callback, nullptr);
#endif
		vkDestroyInstance(instance, nullptr);
		glfwTerminate();
	}

	void init() {
		if (!glfwInit()) {
			throw runtime_error("Failed to initialize GLFW.");
		}
		glfwSetErrorCallback(glfw_error_callback);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		create_instance();
#ifndef NDEBUG
		load_dbg_ext();
		create_dbg_callback();
#endif

		uint32_t n = 0;
		vkEnumeratePhysicalDevices(instance, &n, nullptr);
		physical_devices.resize(n);
		vkEnumeratePhysicalDevices(instance, &n, physical_devices.data());

		atexit(quit);
	}
}
