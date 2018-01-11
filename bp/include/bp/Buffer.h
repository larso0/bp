#ifndef BP_BUFFER_H
#define BP_BUFFER_H

#include "Device.h"
#include "Image.h"
#include "CommandPool.h"

namespace bp
{

class Buffer
{
public:
	Buffer() :
		device{nullptr},
		size{0},
		handle{VK_NULL_HANDLE},
		memoryProperties{0},
		memory{VK_NULL_HANDLE},
		mapped{},
		stagingBuffer{nullptr} {}
	Buffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage,
	       VkMemoryPropertyFlags requiredMemoryProperties,
	       VkMemoryPropertyFlags optimalMemoryProperties = 0) :
		Buffer()
	{
		init(device, size, usage, requiredMemoryProperties, optimalMemoryProperties);
	}
	~Buffer();

	void init(Device& device, VkDeviceSize size, VkBufferUsageFlags usage,
		  VkMemoryPropertyFlags requiredMemoryProperties,
		  VkMemoryPropertyFlags optimalMemoryProperties = 0);

	void* map(VkDeviceSize offset, VkDeviceSize size);
	void unmap(bool writeBack = true, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);
	void updateStagingBuffer(VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);
	void flushStagingBuffer(VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);
	void transfer(VkDeviceSize offset, VkDeviceSize size, const void* data,
		      VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);
	void transfer(Buffer& src, VkDeviceSize srcOffset, VkDeviceSize dstOffset,
		      VkDeviceSize size, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);
	void transfer(Image& src, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);

	operator VkBuffer() { return handle; }

	VkDeviceSize getSize() const { return size; }
	VkBuffer getHandle() { return handle; }
	VkMemoryPropertyFlags getMemoryProperties() const { return memoryProperties; }
	bool isReady() const { return handle != VK_NULL_HANDLE; }

private:
	Device* device;
	VkDeviceSize size;
	VkBuffer handle;
	CommandPool cmdPool;
	VkMemoryPropertyFlags memoryProperties;
	VkDeviceMemory memory;
	VkMappedMemoryRange mapped;
	Buffer* stagingBuffer;

	void assertReady();
};

}

#endif