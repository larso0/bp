#ifndef BP_BUFFER_H
#define BP_BUFFER_H

#include "Device.h"
#include "Image.h"
#include "CommandPool.h"
#include "MemoryAllocator.h"

namespace bp
{

class Buffer
{
public:
	Buffer() :
		device{nullptr},
		size{0},
		handle{VK_NULL_HANDLE},
		memory{nullptr},
		stagingBuffer{nullptr} {}
	Buffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage,
	       VmaMemoryUsage memoryUsage) :
		Buffer()
	{
		init(device, size, usage, memoryUsage);
	}
	~Buffer();

	void init(Device& device, VkDeviceSize size, VkBufferUsageFlags usage,
		  VmaMemoryUsage memoryUsage);

	void* map();
	void createStagingBuffer();
	void freeStagingBuffer();
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
	bool isReady() const { return handle != VK_NULL_HANDLE; }

private:
	Device* device;
	VkDeviceSize size;
	VkBuffer handle;
	CommandPool cmdPool;

	std::shared_ptr<Memory> memory;
	Buffer* stagingBuffer;

	void assertReady();
};

}

#endif