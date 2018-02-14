#ifndef BP_MEMORYALLOCATOR_H
#define BP_MEMORYALLOCATOR_H

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <memory>
#include "Memory.h"

namespace bp
{

class Device;

class MemoryAllocator
{
public:
	MemoryAllocator() :
		physicalDevice{VK_NULL_HANDLE},
		logicalDevice{VK_NULL_HANDLE},
		handle{VK_NULL_HANDLE} {}
	MemoryAllocator(Device& device) :
		MemoryAllocator{}
	{
		init(device);
	}
	~MemoryAllocator();

	void init(Device& device);
	std::shared_ptr<Memory> createBuffer(const VkBufferCreateInfo& bufferInfo,
					     VmaMemoryUsage usage, VkBuffer& buffer,
					     void* opaque = nullptr);
	std::shared_ptr<Memory> createImage(const VkImageCreateInfo& imageInfo,
					    VmaMemoryUsage usage, VkImage& image,
					    void* opaque = nullptr);

	bool isReady() const { return handle != VK_NULL_HANDLE; }

private:
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;
	VmaAllocator handle;
};

}

#endif