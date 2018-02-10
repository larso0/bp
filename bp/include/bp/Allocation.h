#ifndef BP_ALLOCATION_H
#define BP_ALLOCATION_H

#include "Memory.h"
#include <vk_mem_alloc.h>

namespace bp
{

class Allocation : public Memory
{
public:
	Allocation() :
		device{VK_NULL_HANDLE},
		allocator{VK_NULL_HANDLE},
		handle{VK_NULL_HANDLE} {}
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
	bool isReady() const { return handle != VK_NULL_HANDLE; }
	VkDeviceSize getSize() const override { return info.size; }
	void* getMapped() override { return info.pMappedData; }
	void flushMapped() override;

private:
	VkDevice device;
	VmaAllocator allocator;
	VmaAllocation handle;
	VmaAllocationInfo info;
	bool coherent;
};

}

#endif