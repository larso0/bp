#include <bp/device.h>
#include <stdexcept>

using namespace std;

static const char* swapchain_ext_name = "VK_KHR_swapchain";

namespace bp
{
	extern VkInstance instance;
	extern vector<VkPhysicalDevice> physical_devices;

	device::~device()
	{
		if (!m_realized) return;

		if (m_transfer_command_pool != VK_NULL_HANDLE)
			vkDestroyCommandPool(m_logical_handle, m_transfer_command_pool, nullptr);
		if (m_graphics_command_pool != VK_NULL_HANDLE)
			vkDestroyCommandPool(m_logical_handle, m_graphics_command_pool, nullptr);
		if (m_compute_command_pool != VK_NULL_HANDLE)
			vkDestroyCommandPool(m_logical_handle, m_compute_command_pool, nullptr);

		vkDestroyDevice(m_logical_handle, nullptr);
	}

	void device::realize()
	{
		if (m_physical_handle == VK_NULL_HANDLE)
			pick_physical_device();

		vector<VkDeviceQueueCreateInfo> queue_create_infos = setup_queue_create_infos();

		vkGetPhysicalDeviceProperties(m_physical_handle, &m_physical_properties);
		vkGetPhysicalDeviceMemoryProperties(m_physical_handle,
						    &m_physical_memory_properties);

		VkDeviceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		info.queueCreateInfoCount = (uint32_t) queue_create_infos.size();
		info.pQueueCreateInfos = queue_create_infos.data();


		if (m_swapchain_enabled)
		{
			info.enabledExtensionCount = 1;
			info.ppEnabledExtensionNames = &swapchain_ext_name;
		}

		info.pEnabledFeatures = &m_features;

		VkResult result = vkCreateDevice(m_physical_handle, &info,
						 nullptr, &m_logical_handle);
		if (result != VK_SUCCESS)
		{
			throw runtime_error("Failed to create logical device.");
		}

		get_queues();
		create_command_pools();

		m_realized = true;
	}

	int32_t device::find_memory_type(uint32_t desired, VkMemoryPropertyFlags properties) const
	{
		for (uint32_t i = 0; i < m_physical_memory_properties.memoryTypeCount; i++)
			if ((desired & (1 << i)) &&
			    (m_physical_memory_properties.memoryTypes[i].propertyFlags & properties)
			    == properties)
				return i;
		return -1;
	}

	void device::use_physical_device(VkPhysicalDevice device)
	{
		if (m_realized)
		{
			throw runtime_error(
				"Failed to alter physical device, device is already realized.");
		}
		m_physical_handle = device;
	}

	void device::use_surface(VkSurfaceKHR surface, bool swapchain)
	{
		if (m_realized)
		{
			if ((m_queues & VK_QUEUE_GRAPHICS_BIT) == 0)
				throw runtime_error("Realized device is not capable of graphics.");

			VkBool32 supported = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_handle,
							     m_graphics_queue_index, surface,
							     &supported);
			if (!supported)
				throw runtime_error(
					"Realized device does not support given surface.");

			if (swapchain && !m_swapchain_enabled)
				throw runtime_error("Realized device does not support swapchains.");
		} else
		{
			m_swapchain_enabled = swapchain;
		}

		m_surfaces.push_back(surface);
	}

	void device::set_queues(VkQueueFlags queues)
	{
		if (m_realized)
		{
			throw runtime_error(
				"Failed to alter device queues, device is already realized.");
		}
		m_queues = VK_QUEUE_TRANSFER_BIT | queues;
	}

	void device::set_features(const VkPhysicalDeviceFeatures& f)
	{
		if (m_realized)
		{
			throw runtime_error(
				"Failed to alter device features, device is already realized.");
		}
		m_features = f;
	}

	void device::pick_physical_device()
	{
		for (VkPhysicalDevice current : physical_devices)
		{
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(current, &properties);

			uint32_t n;
			vkGetPhysicalDeviceQueueFamilyProperties(current, &n, nullptr);
			vector<VkQueueFamilyProperties> queues(n);
			vkGetPhysicalDeviceQueueFamilyProperties(current, &n, queues.data());

			VkQueueFlags cap = 0;
			for (uint32_t i = 0; i < n; i++)
			{
				VkQueueFlags flags = queues[i].queueFlags;
				if (m_queues & VK_QUEUE_TRANSFER_BIT
				    && !(cap & VK_QUEUE_TRANSFER_BIT)
				    && (flags & VK_QUEUE_TRANSFER_BIT))
				{
					cap |= VK_QUEUE_TRANSFER_BIT;
					m_transfer_queue_index = i;
				}

				if (m_queues & VK_QUEUE_GRAPHICS_BIT
				    && !(cap & VK_QUEUE_GRAPHICS_BIT)
				    && (flags & VK_QUEUE_GRAPHICS_BIT))
				{
					if (!m_surfaces.empty())
					{
						VkBool32 supported = VK_TRUE;
						for (int i = 0;
						     i < m_surfaces.size() && supported; i++)
						{
							vkGetPhysicalDeviceSurfaceSupportKHR(
								current, i,
								m_surfaces[i],
								&supported);
						}

						if (supported)
						{
							cap |= VK_QUEUE_GRAPHICS_BIT;
							m_graphics_queue_index = i;
						}
					} else
					{
						cap |= VK_QUEUE_GRAPHICS_BIT;
						m_graphics_queue_index = i;
					}
				}

				if (m_queues & VK_QUEUE_COMPUTE_BIT
				    && !(cap & VK_QUEUE_COMPUTE_BIT)
				    && flags & VK_QUEUE_COMPUTE_BIT)
				{
					cap |= VK_QUEUE_COMPUTE_BIT;
					m_compute_queue_index = i;
				}

				if (cap == m_queues) break;
			}

			if (cap == m_queues)
			{
				m_physical_handle = current;
				break;
			}
		}

		if (m_physical_handle == VK_NULL_HANDLE)
		{
			throw runtime_error("No suitable physical device found.");
		}
	}

	vector<VkDeviceQueueCreateInfo> device::setup_queue_create_infos()
	{
		vector<VkDeviceQueueCreateInfo> queue_create_infos;
		static const float priority = 1.f;

		if (m_transfer_queue_index > -1)
		{
			queue_create_infos.push_back
				({
					 VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					 nullptr,
					 0,
					 (uint32_t) m_transfer_queue_index,
					 1,
					 &priority
				 });
		}

		if (m_graphics_queue_index > -1 && m_graphics_queue_index != m_transfer_queue_index)
		{
			queue_create_infos.push_back
				({
					 VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					 nullptr,
					 0,
					 (uint32_t) m_graphics_queue_index,
					 1,
					 &priority
				 });
		}

		if (m_compute_queue_index > -1 && m_compute_queue_index != m_transfer_queue_index
		    && m_compute_queue_index != m_graphics_queue_index)
		{
			queue_create_infos.push_back
				({
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

	void device::get_queues()
	{
		if (m_transfer_queue_index > -1)
		{
			vkGetDeviceQueue(m_logical_handle, m_transfer_queue_index, 0,
					 &m_transfer_queue);
		}

		if (m_graphics_queue_index > -1)
		{
			vkGetDeviceQueue(m_logical_handle, m_graphics_queue_index, 0,
					 &m_graphics_queue);
		}

		if (m_compute_queue_index > -1)
		{
			vkGetDeviceQueue(m_logical_handle, m_compute_queue_index, 0,
					 &m_compute_queue);
		}
	}

	void device::create_command_pools()
	{
		VkCommandPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (m_transfer_queue_index > -1)
		{
			info.queueFamilyIndex = (uint32_t) m_transfer_queue_index;
			VkResult result = vkCreateCommandPool(m_logical_handle, &info, nullptr,
							      &m_transfer_command_pool);
			if (result != VK_SUCCESS)
			{
				throw runtime_error("Failed to create transfer command pool.");
			}
		}

		if (m_graphics_queue_index > -1)
		{
			info.queueFamilyIndex = (uint32_t) m_graphics_queue_index;
			VkResult result = vkCreateCommandPool(m_logical_handle, &info, nullptr,
							      &m_graphics_command_pool);
			if (result != VK_SUCCESS)
			{
				throw runtime_error("Failed to create graphics command pool.");
			}
		}

		if (m_compute_queue_index > -1)
		{
			info.queueFamilyIndex = (uint32_t) m_compute_queue_index;
			VkResult result = vkCreateCommandPool(m_logical_handle, &info, nullptr,
							      &m_compute_command_pool);
			if (result != VK_SUCCESS)
			{
				throw runtime_error("Failed to create compute command pool.");
			}
		}
	}
}