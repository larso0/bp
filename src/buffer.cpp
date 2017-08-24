#include <bp/buffer.h>
#include <stdexcept>
#include <cstring>

using namespace std;

namespace bp
{
	buffer::~buffer()
	{
		if (m_realized)
		{
			vkFreeMemory(m_device->logical_handle(), m_memory, nullptr);
			vkDestroyBuffer(m_device->logical_handle(), m_handle, nullptr);
			if (m_staging_buffer) delete m_staging_buffer;
		}
	}

	void buffer::realize()
	{
		VkBufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.size = m_size;
		info.usage = m_usage;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(m_device->logical_handle(), &info, nullptr,
						 &m_handle);
		if (result != VK_SUCCESS)
		{
			throw runtime_error("Failed to create buffer.");
		}

		VkMemoryRequirements mem_requirements = {};
		vkGetBufferMemoryRequirements(m_device->logical_handle(), m_handle,
					      &mem_requirements);


		int32_t mem_type = -1;
		if (m_optimal_properties != 0)
		{
			mem_type = m_device->find_memory_type(mem_requirements.memoryTypeBits,
							      m_optimal_properties);
			m_memory_properties = m_optimal_properties;
		}
		if (mem_type == -1)
		{
			mem_type = m_device->find_memory_type(mem_requirements.memoryTypeBits,
							      m_required_properties);
			m_memory_properties = m_required_properties;
		}
		if (mem_type == -1)
			throw runtime_error("No suitable memory type.");

		VkMemoryAllocateInfo mem_info = {};
		mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_info.allocationSize = mem_requirements.size;
		mem_info.memoryTypeIndex = (uint32_t) mem_type;

		result = vkAllocateMemory(m_device->logical_handle(), &mem_info, nullptr,
					  &m_memory);
		if (result != VK_SUCCESS)
		{
			throw runtime_error("Failed to allocate buffer memory.");
		}

		result = vkBindBufferMemory(m_device->logical_handle(), m_handle, m_memory, 0);
		if (result != VK_SUCCESS)
		{
			throw runtime_error("Failed to bind buffer memory.");
		}

		m_mapped.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		m_mapped.pNext = nullptr;
		m_mapped.memory = m_memory;

		m_realized = true;
	}

	void* buffer::map(VkDeviceSize offset, VkDeviceSize size)
	{
		void* mapped;
		if (!(m_memory_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
		{
			if (!m_staging_buffer)
			{
				m_staging_buffer = new buffer();
				m_staging_buffer->use_device(m_device);
				m_staging_buffer->set_usage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
				m_staging_buffer->set_required_properties(
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
				m_staging_buffer->realize();
			}
			mapped = m_staging_buffer->map(offset, size);
		} else
		{
			VkResult result = vkMapMemory(m_device->logical_handle(), m_memory, offset,
						      size, 0, &mapped);
			if (result != VK_SUCCESS)
				throw runtime_error("Failed to map buffer memory.");
			m_mapped.offset = offset;
			m_mapped.size = size;
		}
		return mapped;
	}

	void buffer::unmap()
	{
		if (m_staging_buffer)
		{
			m_staging_buffer->unmap();
			transfer(*m_staging_buffer, m_staging_buffer->m_mapped.offset,
				 m_staging_buffer->m_mapped.size);
		} else
		{
			vkFlushMappedMemoryRanges(m_device->logical_handle(), 1, &m_mapped);
			vkUnmapMemory(m_device->logical_handle(), m_memory);
		}
	}

	void buffer::transfer(VkDeviceSize offset, VkDeviceSize size, void* data)
	{
		if (size == VK_WHOLE_SIZE)
			size = m_size - offset;
		if (m_staging_buffer)
		{
			void* mapped = m_staging_buffer->map(offset, size);
			memcpy(mapped, data, size);
			m_staging_buffer->unmap();
			transfer(*m_staging_buffer, offset, size);
		} else
		{
			void* mapped = map(offset, size);
			memcpy(mapped, data, size);
			unmap();
		}
	}

	void buffer::transfer(const buffer& from, VkDeviceSize offset, VkDeviceSize size)
	{
		VkCommandBufferAllocateInfo cmd_buffer_info = {};
		cmd_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd_buffer_info.commandPool = m_device->transfer_command_pool();
		cmd_buffer_info.commandBufferCount = 1;

		VkCommandBuffer cmd_buffer;
		vkAllocateCommandBuffers(m_device->logical_handle(), &cmd_buffer_info, &cmd_buffer);

		VkCommandBufferBeginInfo begin_info = {};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmd_buffer, &begin_info);

		VkBufferCopy copy_region = {};
		copy_region.srcOffset = 0;
		copy_region.dstOffset = offset;

		if (size == VK_WHOLE_SIZE)
			copy_region.size = m_size - offset;
		else
			copy_region.size = size;

		vkCmdCopyBuffer(cmd_buffer, from.handle(), m_handle, 1,
				&copy_region);

		vkEndCommandBuffer(cmd_buffer);

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cmd_buffer;

		vkQueueSubmit(m_device->transfer_queue(), 1, &submit_info,
			      VK_NULL_HANDLE);
		vkQueueWaitIdle(m_device->transfer_queue());

		vkFreeCommandBuffers(m_device->logical_handle(), m_device->transfer_command_pool(),
				     1, &cmd_buffer);
	}

	void buffer::use_device(std::shared_ptr<device> d)
	{
		if (m_realized)
			throw runtime_error("Failed to alter device, buffer already realized.");
		m_device = d;
	}

	void buffer::set_usage(VkBufferUsageFlags usage)
	{
		if (m_realized)
			throw runtime_error("Failed to alter usage, buffer already realized.");
		m_usage = usage;
	}

	void buffer::set_required_properties(VkMemoryPropertyFlags properties)
	{
		if (m_realized)
			throw runtime_error("Failed to alter properties, buffer already realized.");
		m_required_properties = properties;
	}

	void buffer::set_optimal_properties(VkMemoryPropertyFlags properties)
	{
		if (m_realized)
			throw runtime_error("Failed to alter properties, buffer already realized.");
		m_optimal_properties = properties;
	}

	void buffer::set_size(VkDeviceSize size)
	{
		if (m_realized)
			throw runtime_error("Failed to alter size, buffer already realized.");
		m_size = size;
	}
}