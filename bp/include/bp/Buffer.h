#ifndef BP_BUFFER_H
#define BP_BUFFER_H

#include "Device.h"
#include "Pointer.h"
#include "Image.h"

namespace bp
{

class Buffer
{
public:
	Buffer() :
		device{nullptr},
		size{0},
		handle{VK_NULL_HANDLE},
		cmdPool{VK_NULL_HANDLE},
		memoryProperties{0},
		memory{VK_NULL_HANDLE},
		mapped{},
		stagingBuffer{nullptr} {}
	Buffer(NotNull<Device> device, VkDeviceSize size, VkBufferUsageFlags usage,
	       VkMemoryPropertyFlags requiredMemoryProperties,
	       VkMemoryPropertyFlags optimalMemoryProperties = 0) :
		Buffer()
	{
		init(device, size, usage, requiredMemoryProperties, optimalMemoryProperties);
	}
	~Buffer();

	void init(NotNull<Device> device, VkDeviceSize size, VkBufferUsageFlags usage,
		  VkMemoryPropertyFlags requiredMemoryProperties,
		  VkMemoryPropertyFlags optimalMemoryProperties = 0);

	void* map(VkDeviceSize offset, VkDeviceSize size);
	void unmap(bool writeBack = true);
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
	VkCommandPool cmdPool;
	VkMemoryPropertyFlags memoryProperties;
	VkDeviceMemory memory;
	VkMappedMemoryRange mapped;
	Buffer* stagingBuffer;

	void createCommandPool();
	void assertReady();
};

}

#endif