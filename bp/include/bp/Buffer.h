#ifndef BP_BUFFER_H
#define BP_BUFFER_H

#include <vulkan/vulkan.h>

namespace bp
{

class Buffer
{
public:
	Buffer() :
		physicalDevice(VK_NULL_HANDLE),
		device(VK_NULL_HANDLE),
		transferQueueFamilyIndex(0),
		transferQueue(VK_NULL_HANDLE),
		usage(0),
		requiredMemoryProperties(0), optimalMemoryProperties(0),
		size(0),
		handle(VK_NULL_HANDLE),
		cmdPool(VK_NULL_HANDLE),
		memoryProperties(0),
		memory(VK_NULL_HANDLE),
		mapped({}),
		stagingBuffer(nullptr) {}
	~Buffer();

	void init();
	void* map(VkDeviceSize offset, VkDeviceSize size);
	void unmap();
	void transfer(VkDeviceSize offset, VkDeviceSize size, const void* data,
		      VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);
	void transfer(Buffer& src, VkDeviceSize srcOffset, VkDeviceSize dstOffset,
		      VkDeviceSize size, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);

	void setDevice(VkPhysicalDevice physical, VkDevice logical);
	void setTransferQueue(uint32_t queueFamilyIndex, VkQueue queue);
	void setUsage(VkBufferUsageFlags usage);
	void setRequiredMemoryProperties(VkMemoryPropertyFlags properties);
	void setOptimalMemoryProperties(VkMemoryPropertyFlags properties);
	void setSize(VkDeviceSize size);

	VkDeviceSize getSize() const { return size; }
	VkBuffer getHandle() { return handle; }
	VkMemoryPropertyFlags getMemoryProperties() const { return memoryProperties; }
	bool isReady() const { return handle != VK_NULL_HANDLE; }
	bool isMappable() const
	{
		return (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) ||
		       transferQueue != VK_NULL_HANDLE;
	}

private:
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	uint32_t transferQueueFamilyIndex;
	VkQueue transferQueue;
	VkBufferUsageFlags usage;
	VkMemoryPropertyFlags requiredMemoryProperties, optimalMemoryProperties;
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