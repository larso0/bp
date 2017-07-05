#ifndef BP_DEVICE_H
#define BP_DEVICE_H

#include <vulkan/vulkan.h>
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
			m_physical_handle(VK_NULL_HANDLE),
			m_logical_handle(VK_NULL_HANDLE),
			m_physical_properties({}),
			m_physical_memory_properties({}),
			m_physical_features({}) {}

		void capabilities(capability c);

		void features(feature f);

		capability capabilities() const { return m_capabilities; }

		feature features() const { return m_features; }

		VkPhysicalDevice physical_handle() const { return m_physical_handle; }

		VkDevice logical_handle() { return m_logical_handle; }

	private:
		capability m_capabilities;
		feature m_features;

		VkPhysicalDevice m_physical_handle;
		VkDevice m_logical_handle;

		VkPhysicalDeviceProperties m_physical_properties;
		VkPhysicalDeviceMemoryProperties m_physical_memory_properties;
		VkPhysicalDeviceFeatures m_physical_features;

	};
}

BP_DECLARE_BITMASK_OPERATORS(bp::device::capability)

BP_DECLARE_BITMASK_OPERATORS(bp::device::feature)

#endif
