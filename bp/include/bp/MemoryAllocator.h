#ifndef BP_MEMORYALLOCATOR_H
#define BP_MEMORYALLOCATOR_H

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <memory>
#include "Memory.h"

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
	std::shared_ptr<Allocation> createBuffer(const VkBufferCreateInfo& bufferInfo, VmaMemoryUsage usage,
						 VkBuffer& buffer);
	std::shared_ptr<Allocation> createImage(const VkImageCreateInfo& bufferInfo, VmaMemoryUsage usage,
						VkImage& image);

	bool isReady() const { return handle != VK_NULL_HANDLE; }

private:
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;
	VmaAllocator handle;
};

}

#endif