#ifndef BP_MEMORYALLOCATOR_H
#define BP_MEMORYALLOCATOR_H

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "Memory.h"

namespace bp
{

class Allocation : public Memory
{
public:
	Allocation() :
		device{VK_NULL_HANDLE},
		allocator{VMA_NULL},
		handle{VMA_NULL} {}
	Allocation(VkDevice device, VmaAllocator allocator, VmaAllocation handle,
		   VmaAllocationInfo& info) :
		device{device},
		allocator{allocator},
		handle{handle},
		info{info}
	{
		VkMemoryPropertyFlags memFlags;
		vmaGetMemoryTypeProperties(allocator, info.memoryType, &memFlags);
		coherent = static_cast<bool>(memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}
	Allocation(Allocation&& other);
	~Allocation();

	bool isMapped() const override { return info.pMappedData != nullptr; }
	bool isReady() const { return handle != VMA_NULL; }
	void* getMapped() override { return info.pMappedData; }
	void flushMapped() override;

private:
	VkDevice device;
	VmaAllocator allocator;
	VmaAllocation handle;
	VmaAllocationInfo info;
	bool coherent;
};

class MemoryAllocator
{
public:
	MemoryAllocator() :
		physicalDevice{VK_NULL_HANDLE},
		logicalDevice{VK_NULL_HANDLE},
		handle{VMA_NULL} {}
	MemoryAllocator(VkPhysicalDevice physicalDevice, VkDevice logicalDevice) :
		MemoryAllocator{}
	{
		init(physicalDevice, logicalDevice);
	}
	~MemoryAllocator();

	void init(VkPhysicalDevice physicalDevice, VkDevice logicalDevice);
	Allocation createBuffer(const VkBufferCreateInfo& bufferInfo, VmaMemoryUsage usage,
				VkBuffer& buffer);
	Allocation createImage(const VkImageCreateInfo& bufferInfo, VmaMemoryUsage usage,
			       VkImage& image);

	bool isReady() const { return handle != VMA_NULL; }

private:
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;
	VmaAllocator handle;
};

}

#endif