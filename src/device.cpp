#include <bp/device.h>
#include <stdexcept>

using namespace std;

static const char* swapchain_ext_name = "VK_KHR_swapchain";

namespace bp {
	extern VkInstance instance;
	extern vector<VkPhysicalDevice> physical_devices;

	void device::realize() {
		pick_physical_device();

		vector<VkDeviceQueueCreateInfo> queue_create_infos = setup_queue_create_infos();

		vkGetPhysicalDeviceProperties(m_physical_handle, &m_physical_properties);
		vkGetPhysicalDeviceMemoryProperties(m_physical_handle, &m_physical_memory_properties);

		VkDeviceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		info.queueCreateInfoCount = (uint32_t) queue_create_infos.size();
		info.pQueueCreateInfos = queue_create_infos.data();

		VkPhysicalDeviceFeatures features = {};

		if ((m_capabilities & capability::graphics) != capability::none) {
			info.enabledExtensionCount = 1;
			info.ppEnabledExtensionNames = &swapchain_ext_name;
		}

		if ((m_features & feature::sampler_anisotropy) != feature::none) {
			features.samplerAnisotropy = VK_TRUE;
		}

		if ((m_features & feature::shader_clip_distance) != feature::none) {
			features.shaderClipDistance = VK_TRUE;
		}

		info.pEnabledFeatures = &features;

		VkResult result = vkCreateDevice(m_physical_handle, &info,
		                                 nullptr, &m_logical_handle);
		if (result != VK_SUCCESS) {
			throw runtime_error("Failed to create logical device.");
		}

		get_queues();
		create_command_pools();

		m_realized = true;
	}

	void device::use_surface(VkSurfaceKHR s) {
		if (m_realized) {
			if ((m_capabilities & capability::graphics) == capability::none) {
				throw runtime_error("Realized device is not capable of graphics.");
			}

			VkBool32 supported = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_handle, m_graphics_queue_index, s, &supported);
			if (!supported) {
				throw runtime_error("Realized device does not support given surface.");
			}
		}
		m_surface = s;
	}

	void device::capabilities(capability c) {
		if (m_realized) {
			throw runtime_error("Failed to alter device capabilities, device is already realized.");
		}
		m_capabilities = c;
	}

	void device::features(feature f) {
		if (m_realized) {
			throw runtime_error("Failed to alter device features, device is already realized.");
		}
		m_features = f;
	}

	void device::pick_physical_device() {
		for (VkPhysicalDevice current : physical_devices) {
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(current, &properties);

			uint32_t n;
			vkGetPhysicalDeviceQueueFamilyProperties(current, &n, nullptr);
			vector<VkQueueFamilyProperties> queues(n);
			vkGetPhysicalDeviceQueueFamilyProperties(current, &n, queues.data());

			capability cap = capability::none;
			for (uint32_t i = 0; i < n; i++) {
				VkQueueFlags flags = queues[i].queueFlags;
				if ((m_capabilities & capability::transfer) != capability::none
				    && (cap & capability::transfer) == capability::none
				    && (flags & VK_QUEUE_TRANSFER_BIT)) {
					cap = cap | capability::transfer;
					m_transfer_queue_index = i;
				}

				if ((m_capabilities & capability::graphics) != capability::none
				    && (cap & capability::graphics) == capability::none
				    && (flags & VK_QUEUE_GRAPHICS_BIT)) {
					if (m_surface != VK_NULL_HANDLE) {
						VkBool32 supported;
						vkGetPhysicalDeviceSurfaceSupportKHR(current, i, m_surface,
						                                     &supported);
						if (supported) {
							cap = cap | capability::graphics;
							m_graphics_queue_index = i;
						}
					} else {
						cap = cap | capability::graphics;
						m_graphics_queue_index = i;
					}
				}

				if ((m_capabilities & capability::compute) != capability::none
				    && (cap & capability::compute) == capability::none
				    && (flags & VK_QUEUE_COMPUTE_BIT)) {
					cap = cap | capability::compute;
					m_compute_queue_index = i;
				}

				if (cap == m_capabilities) break;
			}

			if (cap == m_capabilities) {
				m_physical_handle = current;
				break;
			}
		}

		if (m_physical_handle == VK_NULL_HANDLE) {
			throw runtime_error("No suitable physical device found.");
		}
	}

	vector<VkDeviceQueueCreateInfo> device::setup_queue_create_infos() {
		vector<VkDeviceQueueCreateInfo> queue_create_infos;
		static const float priority = 1.f;

		if (m_transfer_queue_index > -1) {
			queue_create_infos.push_back({
				                             VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				                             nullptr,
				                             0,
				                             (uint32_t) m_transfer_queue_index,
				                             1,
				                             &priority
			                             });
		}

		if (m_graphics_queue_index > -1 && m_graphics_queue_index != m_transfer_queue_index) {
			queue_create_infos.push_back({
				                             VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				                             nullptr,
				                             0,
				                             (uint32_t) m_graphics_queue_index,
				                             1,
				                             &priority
			                             });
		}

		if (m_compute_queue_index > -1 && m_compute_queue_index != m_transfer_queue_index
		    && m_compute_queue_index != m_graphics_queue_index) {
			queue_create_infos.push_back({
				                             VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				                             nullptr,
				                             0,
				                             (uint32_t) m_compute_queue_index,
				                             1,
				                             &priority
			                             });
		}

		return queue_create_infos;
	}

	void device::get_queues() {
		if (m_transfer_queue_index > -1) {
			vkGetDeviceQueue(m_logical_handle, m_transfer_queue_index, 0, &m_transfer_queue);
		}

		if (m_graphics_queue_index > -1) {
			vkGetDeviceQueue(m_logical_handle, m_graphics_queue_index, 0, &m_graphics_queue);
		}

		if (m_compute_queue_index > -1) {
			vkGetDeviceQueue(m_logical_handle, m_compute_queue_index, 0, &m_compute_queue);
		}
	}

	void device::create_command_pools() {
		VkCommandPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (m_transfer_queue_index > -1) {
			info.queueFamilyIndex = (uint32_t) m_transfer_queue_index;
			VkResult result = vkCreateCommandPool(m_logical_handle, &info, nullptr,
			                                      &m_transfer_command_pool);
			if (result != VK_SUCCESS) {
				throw runtime_error("Failed to create transfer command pool.");
			}
		}

		if (m_graphics_queue_index > -1) {
			info.queueFamilyIndex = (uint32_t) m_graphics_queue_index;
			VkResult result = vkCreateCommandPool(m_logical_handle, &info, nullptr,
			                                      &m_graphics_command_pool);
			if (result != VK_SUCCESS) {
				throw runtime_error("Failed to create graphics command pool.");
			}
		}

		if (m_compute_queue_index > -1) {
			info.queueFamilyIndex = (uint32_t) m_compute_queue_index;
			VkResult result = vkCreateCommandPool(m_logical_handle, &info, nullptr,
			                                      &m_compute_command_pool);
			if (result != VK_SUCCESS) {
				throw runtime_error("Failed to create compute command pool.");
			}
		}
	}
}

BP_DEFINE_BITMASK_OPERATORS(bp::device::capability)

BP_DEFINE_BITMASK_OPERATORS(bp::device::feature)