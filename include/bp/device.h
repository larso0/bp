#ifndef BP_DEVICE_H
#define BP_DEVICE_H

#include <vulkan/vulkan.h>
#include <vector>

namespace bp
{
	class device
	{
	public:
		device() :
			m_queues(VK_QUEUE_TRANSFER_BIT),
			m_swapchain_enabled(false),
			m_features({}),
			m_realized(false),
			m_physical_handle(VK_NULL_HANDLE),
			m_logical_handle(VK_NULL_HANDLE),
			m_physical_properties({}),
			m_physical_memory_properties({}),
			m_transfer_queue_index(-1),
			m_graphics_queue_index(-1),
			m_compute_queue_index(-1),
			m_transfer_queue(VK_NULL_HANDLE),
			m_graphics_queue(VK_NULL_HANDLE),
			m_compute_queue(VK_NULL_HANDLE),
			m_transfer_command_pool(VK_NULL_HANDLE),
			m_graphics_command_pool(VK_NULL_HANDLE),
			m_compute_command_pool(VK_NULL_HANDLE) {}

		~device();

		void realize();
		int32_t find_memory_type(uint32_t desired, VkMemoryPropertyFlags properties) const;

		void use_physical_device(VkPhysicalDevice device);
		void use_surface(VkSurfaceKHR s, bool swapchain = true);

		void set_queues(VkQueueFlags queues);
		void set_features(const VkPhysicalDeviceFeatures& f);

		bool is_realized() const { return m_realized; }

		VkPhysicalDevice physical_handle() const { return m_physical_handle; }

		VkDevice logical_handle() { return m_logical_handle; }

		VkQueueFlags queues() const { return m_queues; }

		const VkPhysicalDeviceFeatures& features() const { return m_features; }

		const VkPhysicalDeviceProperties& physical_properties() const
		{
			return m_physical_properties;
		}

		const VkPhysicalDeviceMemoryProperties physical_memory_properties() const
		{
			return m_physical_memory_properties;
		}

		VkQueue transfer_queue() { return m_transfer_queue; }
		VkQueue graphics_queue() { return m_graphics_queue; }
		VkQueue compute_queue() { return m_compute_queue; }

		VkCommandPool transfer_command_pool() { return m_transfer_command_pool; }
		VkCommandPool graphics_command_pool() { return m_graphics_command_pool; }
		VkCommandPool compute_command_pool() { return m_compute_command_pool; }

	private:
		bool m_realized;

		VkPhysicalDevice m_physical_handle;
		VkDevice m_logical_handle;

		VkQueueFlags m_queues;
		bool m_swapchain_enabled;
		VkPhysicalDeviceFeatures m_features;
		VkPhysicalDeviceProperties m_physical_properties;
		VkPhysicalDeviceMemoryProperties m_physical_memory_properties;

		std::vector<VkSurfaceKHR> m_surfaces;

		int m_transfer_queue_index, m_graphics_queue_index, m_compute_queue_index;
		VkQueue m_transfer_queue, m_graphics_queue, m_compute_queue;
		VkCommandPool m_transfer_command_pool, m_graphics_command_pool, m_compute_command_pool;

		void pick_physical_device();

		std::vector<VkDeviceQueueCreateInfo> setup_queue_create_infos();

		void get_queues();

		void create_command_pools();
	};
}

#endif
