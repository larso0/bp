#ifndef BP_IMAGE_H
#define BP_IMAGE_H

#include "vk_includes.h"
#include "device.h"
#include <memory>

namespace bp
{
	class image
	{
	public:
		image() :
			m_realized(false),
			m_handle(VK_NULL_HANDLE),
			m_width(0),
			m_height(0),
			m_format(VK_FORMAT_UNDEFINED),
			m_tiling(VK_IMAGE_TILING_LINEAR),
			m_layout(VK_IMAGE_LAYOUT_UNDEFINED),
			m_usage(0),
			m_memory_properties(0),
			m_memory_size(0),
			m_memory(VK_NULL_HANDLE),
			m_mapped({}) {}
		~image();

		void realize();
		void* map(VkDeviceSize offset, VkDeviceSize size);
		void unmap();
		void record_transition(VkImageLayout dst_layout, VkCommandBuffer cmd_buffer);
		void record_transfer(image& from, VkCommandBuffer cmd_buffer);
		void transition(VkImageLayout dst_layout);
		void transfer(image& from);
		void transfer_transition(image& from, VkImageLayout dst_layout);

		void use_device(std::shared_ptr<device> d);
		void set_size(uint32_t width, uint32_t height);
		void set_format(VkFormat format);
		void set_tiling(VkImageTiling tiling);
		void set_layout(VkImageLayout layout);
		void set_usage(VkImageUsageFlags usage);
		void set_memory_properties(VkMemoryPropertyFlags properties);

		bool is_realized() const { return m_realized; }
		VkImage handle() const { return m_handle; }
		uint32_t width() const { return m_width; }
		uint32_t height() const { return m_height; }
		VkFormat format() const { return m_format; }
		VkImageTiling tiling() const { return m_tiling; }
		VkImageLayout layout() const { return m_layout; }

	private:
		bool m_realized;
		VkImage m_handle;

		std::shared_ptr<device> m_device;

		uint32_t m_width, m_height;
		VkFormat m_format;
		VkImageTiling m_tiling;
		VkImageLayout m_layout;
		VkImageUsageFlags m_usage;
		VkMemoryPropertyFlags m_memory_properties;
		VkDeviceSize m_memory_size;
		VkDeviceMemory m_memory;
		VkMappedMemoryRange m_mapped;
	};
}

#endif