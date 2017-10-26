#ifndef BP_BUFFER_H
#define BP_BUFFER_H

#include "Device.h"
#include <vulkan/vulkan.h>

namespace bp
{

class Buffer
{
public:
	Buffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage,
	       VkMemoryPropertyFlags requiredMemoryProperties,
	       VkMemoryPropertyFlags optimalMemoryProperties = 0);
	~Buffer();

	void* map(VkDeviceSize offset, VkDeviceSize size);
	void unmap();
	void transfer(VkDeviceSize offset, VkDeviceSize size, const void* data,
		      VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);
	void transfer(Buffer& src, VkDeviceSize srcOffset, VkDeviceSize dstOffset,
		      VkDeviceSize size, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);

	operator VkBuffer() { return handle; }

	VkDeviceSize getSize() const { return size; }
	VkBuffer getHandle() { return handle; }
	VkMemoryPropertyFlags getMemoryProperties() const { return memoryProperties; }

private:
	Device& device;
	VkDeviceSize size;
	VkBuffer handle;
	VkCommandPool cmdPool;
	VkMemoryPropertyFlags memoryProperties;
	VkDeviceMemory memory;
	VkMappedMemoryRange mapped;
	Buffer* stagingBuffer;

	void createCommandPool();
};

}

#endif