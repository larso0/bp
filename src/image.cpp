#include <bp/image.h>
#include <stdexcept>

using namespace std;

namespace bp
{
	image::~image()
	{
		vkFreeMemory(m_device->logical_handle(), m_memory, nullptr);
		vkDestroyImage(m_device->logical_handle(), m_handle, nullptr);
	}

	void image::realize()
	{
		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.extent.width = m_width;
		info.extent.height = m_height;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.format = m_format;
		info.tiling = m_tiling;
		info.initialLayout = m_layout;
		info.usage = m_usage;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateImage(m_device->logical_handle(), &info, nullptr,
						&m_handle);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create image.");

		VkMemoryRequirements requirements;
		vkGetImageMemoryRequirements(m_device->logical_handle(), m_handle, &requirements);

		int32_t mem_type = m_device->find_memory_type(requirements.memoryTypeBits,
							      m_memory_properties);
		if (mem_type == -1)
			throw runtime_error("No suitable memory type.");

		VkMemoryAllocateInfo mem_info = {};
		mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		mem_info.allocationSize = requirements.size;
		mem_info.memoryTypeIndex = (uint32_t) mem_type;

		result = vkAllocateMemory(m_device->logical_handle(), &mem_info, nullptr,
					  &m_memory);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to allocate image memory.");

		result = vkBindImageMemory(m_device->logical_handle(), m_handle, m_memory, 0);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to bind image memory.");

		m_mapped.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		m_mapped.pNext = nullptr;
		m_mapped.memory = m_memory;
		m_memory_size = requirements.size;
	}

	void* image::map(VkDeviceSize offset, VkDeviceSize size)
	{
		void* mapped;
		VkResult result = vkMapMemory(m_device->logical_handle(), m_memory, offset, size, 0,
					      &mapped);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to map image memory.");
		m_mapped.offset = offset;
		m_mapped.size = size;
		return mapped;
	}

	void image::unmap()
	{
		vkFlushMappedMemoryRanges(m_device->logical_handle(), 1, &m_mapped);
		vkUnmapMemory(m_device->logical_handle(), m_memory);
	}

	void image::record_transition(VkImageLayout dst_layout, VkCommandBuffer cmd_buffer)
	{
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = m_layout;
		barrier.newLayout = dst_layout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_handle;
		barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;

		VkPipelineStageFlags dst_stage_mask = 0;

		if (barrier.oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED &&
		    barrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} else if (barrier.oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED &&
			   barrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} else if (barrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
			   barrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} else if (barrier.newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
						VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			dst_stage_mask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		} else
		{
			throw runtime_error("Unsupported image layout transition.");
		}

		vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				     dst_stage_mask, 0, 0, nullptr,
				     0, nullptr, 1, &barrier);

		m_layout = dst_layout;
	}

	void image::record_transfer(image& from, VkCommandBuffer cmd_buffer)
	{
		from.record_transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, cmd_buffer);
		record_transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cmd_buffer);

		VkImageSubresourceLayers sub_resource = {};
		sub_resource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		sub_resource.baseArrayLayer = 0;
		sub_resource.mipLevel = 0;
		sub_resource.layerCount = 1;

		VkImageCopy region = {};
		region.srcSubresource = sub_resource;
		region.dstSubresource = sub_resource;
		region.srcOffset = {0, 0, 0};
		region.dstOffset = {0, 0, 0};
		region.extent.width = m_width;
		region.extent.height = m_height;
		region.extent.depth = 1;

		vkCmdCopyImage(cmd_buffer, from.handle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			       m_handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}

	void image::transition(VkImageLayout dst_layout)
	{
		VkCommandBufferAllocateInfo cmd_buffer_info = {};
		cmd_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd_buffer_info.commandPool = m_device->transfer_command_pool();
		cmd_buffer_info.commandBufferCount = 1;

		VkCommandBuffer cmd_buffer;
		vkAllocateCommandBuffers(m_device->logical_handle(), &cmd_buffer_info, &cmd_buffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmd_buffer, &beginInfo);

		record_transition(dst_layout, cmd_buffer);

		vkEndCommandBuffer(cmd_buffer);

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cmd_buffer;

		vkQueueSubmit(m_device->transfer_queue(), 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_device->transfer_queue());

		vkFreeCommandBuffers(m_device->logical_handle(), m_device->transfer_command_pool(),
				     1, &cmd_buffer);
	}

	void image::transfer(image& from)
	{
		VkCommandBufferAllocateInfo cmd_buffer_info = {};
		cmd_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd_buffer_info.commandPool = m_device->transfer_command_pool();
		cmd_buffer_info.commandBufferCount = 1;

		VkCommandBuffer cmd_buffer;
		vkAllocateCommandBuffers(m_device->logical_handle(), &cmd_buffer_info, &cmd_buffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmd_buffer, &beginInfo);

		record_transfer(from, cmd_buffer);

		vkEndCommandBuffer(cmd_buffer);

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cmd_buffer;

		vkQueueSubmit(m_device->transfer_queue(), 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_device->transfer_queue());

		vkFreeCommandBuffers(m_device->logical_handle(), m_device->transfer_command_pool(),
				     1, &cmd_buffer);
	}

	void image::transfer_transition(image& from, VkImageLayout dst_layout)
	{
		VkCommandBufferAllocateInfo cmd_buffer_info = {};
		cmd_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_buffer_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd_buffer_info.commandPool = m_device->transfer_command_pool();
		cmd_buffer_info.commandBufferCount = 1;

		VkCommandBuffer cmd_buffer;
		vkAllocateCommandBuffers(m_device->logical_handle(), &cmd_buffer_info, &cmd_buffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmd_buffer, &beginInfo);

		record_transfer(from, cmd_buffer);
		record_transition(dst_layout, cmd_buffer);

		vkEndCommandBuffer(cmd_buffer);

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &cmd_buffer;

		vkQueueSubmit(m_device->transfer_queue(), 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_device->transfer_queue());

		vkFreeCommandBuffers(m_device->logical_handle(), m_device->transfer_command_pool(),
				     1, &cmd_buffer);
	}

	void image::use_device(std::shared_ptr<device> d)
	{
		if (m_realized)
			throw runtime_error("Failed to alter device, image already realized.");
		m_device = d;
	}

	void image::set_size(uint32_t width, uint32_t height)
	{
		if (m_realized)
			throw runtime_error("Failed to alter size, image already realized.");
		m_width = width;
		m_height = height;
	}

	void image::set_format(VkFormat format)
	{
		if (m_realized)
			throw runtime_error("Failed to alter format, image already realized.");
		m_format = format;
	}

	void image::set_tiling(VkImageTiling tiling)
	{
		if (m_realized)
			throw runtime_error("Failed to alter tiling, image already realized.");
		m_tiling = tiling;
	}

	void image::set_layout(VkImageLayout layout)
	{
		if (m_realized) transition(layout);
		else m_layout = layout;
	}

	void image::set_usage(VkImageUsageFlags usage)
	{
		if (m_realized)
			throw runtime_error("Failed to alter usage, image already realized.");
		m_usage = usage;
	}

	void image::set_memory_properties(VkMemoryPropertyFlags properties)
	{
		if (m_realized)
		{
			throw runtime_error(
				"Failed to alter memory properties, image already realized;");
		}
		m_memory_properties = properties;
	}
}