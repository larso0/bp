#ifndef BP_DEVICE_H
#define BP_DEVICE_H

#include <vulkan/vulkan.h>
#include <vector>
#include "bitmask_operators.h"

namespace bp {
	class device {
	public:
		enum class capability : int {
			none = 0x00,
			transfer = 0x01,
			graphics = 0x02,
			compute = 0x04
		};

		enum class feature : int {
			none = 0x0000,
			shader_clip_distance = 0x0001,
			sampler_anisotropy = 0x0002
		};

		device() :
			m_capabilities(capability::none),
			m_features(feature::none),
			m_realized(false),
			m_physical_handle(VK_NULL_HANDLE),
			m_logical_handle(VK_NULL_HANDLE),
			m_physical_properties({}),
			m_physical_memory_properties({}),
			m_surface(VK_NULL_HANDLE),
			m_transfer_queue_index(-1),
			m_graphics_queue_index(-1),
			m_compute_queue_index(-1),
			m_transfer_queue(VK_NULL_HANDLE),
			m_graphics_queue(VK_NULL_HANDLE),
			m_compute_queue(VK_NULL_HANDLE),
			m_transfer_command_pool(VK_NULL_HANDLE),
			m_graphics_command_pool(VK_NULL_HANDLE),
			m_compute_command_pool(VK_NULL_HANDLE) {}

		void realize();

		void use_surface(VkSurfaceKHR s);

		void capabilities(capability c);

		void features(feature f);

		capability capabilities() const { return m_capabilities; }

		feature features() const { return m_features; }

		VkPhysicalDevice physical_handle() const { return m_physical_handle; }

		VkDevice logical_handle() { return m_logical_handle; }

		const VkPhysicalDeviceProperties& physical_properties() const { return m_physical_properties; }

		const VkPhysicalDeviceMemoryProperties physical_memory_properties() const {
			return m_physical_memory_properties;
		}

		VkQueue transfer_queue() { return m_transfer_queue; }

		VkQueue graphics_queue() { return m_graphics_queue; }

		VkQueue compute_queue() { return m_compute_queue; }

		VkCommandPool transfer_command_pool() { return m_transfer_command_pool; }

		VkCommandPool graphics_command_pool() { return m_graphics_command_pool; }

		VkCommandPool compute_command_pool() { return m_compute_command_pool; }

	private:
		capability m_capabilities;
		feature m_features;
		bool m_realized;

		VkPhysicalDevice m_physical_handle;
		VkDevice m_logical_handle;

		VkPhysicalDeviceProperties m_physical_properties;
		VkPhysicalDeviceMemoryProperties m_physical_memory_properties;

		VkSurfaceKHR m_surface;

		int m_transfer_queue_index, m_graphics_queue_index, m_compute_queue_index;
		VkQueue m_transfer_queue, m_graphics_queue, m_compute_queue;
		VkCommandPool m_transfer_command_pool, m_graphics_command_pool, m_compute_command_pool;

		void pick_physical_device();

		std::vector<VkDeviceQueueCreateInfo> setup_queue_create_infos();

		void get_queues();

		void create_command_pools();
	};
}

BP_DECLARE_BITMASK_OPERATORS(bp::device::capability)

BP_DECLARE_BITMASK_OPERATORS(bp::device::feature)

#endif
