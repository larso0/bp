#ifndef BP_MEMORYALLOCATOR_H
#define BP_MEMORYALLOCATOR_H

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <memory>
#include "Memory.h"
#include "Allocation.h"

namespace bp
{

class MemoryAllocator
{
public:
	MemoryAllocator() :
		physicalDevice{VK_NULL_HANDLE},
		logicalDevice{VK_NULL_HANDLE},
		handle{VK_NULL_HANDLE} {}
	MemoryAllocator(VkPhysicalDevice physicalDevice, VkDevice logicalDevice) :
		MemoryAllocator{}
	{
		init(physicalDevice, logicalDevice);
	}
	~MemoryAllocator();

	void init(VkPhysicalDevice physicalDevice, VkDevice logicalDevice);
	std::shared_ptr<Allocation> createBuffer(const VkBufferCreateInfo& bufferInfo,
						 VmaMemoryUsage usage, VkBuffer& buffer);
	std::shared_ptr<Allocation> createImage(const VkImageCreateInfo& bufferInfo,
						VmaMemoryUsage usage, VkImage& image);

	bool isReady() const { return handle != VK_NULL_HANDLE; }

private:
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;
	VmaAllocator handle;
};

}

#endif