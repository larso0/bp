#ifndef BP_BUFFER_H
#define BP_BUFFER_H

#include "vk_includes.h"
#include "device.h"
#include <memory>

namespace bp
{
	class buffer
	{
	public:
		buffer() :
			m_realized(false),
			m_handle(VK_NULL_HANDLE),
			m_usage(0),
			m_required_properties(0),
			m_optimal_properties(0),
			m_size(0),
			m_memory_properties(0),
			m_memory(VK_NULL_HANDLE),
			m_mapped({}),
			m_staging_buffer(nullptr) {}
		~buffer();

		void realize();

		void* map(VkDeviceSize offset, VkDeviceSize size);
		void unmap();
		void transfer(VkDeviceSize offset, VkDeviceSize size, void* data);
		void transfer(const buffer& from, VkDeviceSize offset, VkDeviceSize size);

		void use_device(std::shared_ptr<device> d);
		void set_usage(VkBufferUsageFlags usage);
		void set_required_properties(VkMemoryPropertyFlags properties);
		void set_optimal_properties(VkMemoryPropertyFlags properties);
		void set_size(VkDeviceSize size);

		bool is_realized() const { return m_realized; }
		VkBuffer handle() const { return m_handle; }
		VkMemoryPropertyFlags memory_properties() const { return m_memory_properties; }

	private:
		bool m_realized;
		VkBuffer m_handle;

		std::shared_ptr<device> m_device;

		VkBufferUsageFlags m_usage;
		VkMemoryPropertyFlags m_required_properties, m_optimal_properties;

		VkDeviceSize m_size;
		VkMemoryPropertyFlags m_memory_properties;
		VkDeviceMemory m_memory;
		VkMappedMemoryRange m_mapped;
		buffer* m_staging_buffer;

	};
}

#endif
