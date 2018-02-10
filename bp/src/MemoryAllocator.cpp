#include <bp/MemoryAllocator.h>
#include <stdexcept>

using namespace std;

namespace bp
{

MemoryAllocator::~MemoryAllocator()
{
	if (isReady())
		vmaDestroyAllocator(handle);
}

void MemoryAllocator::init(VkPhysicalDevice physicalDevice, VkDevice logicalDevice)
{
	MemoryAllocator::physicalDevice = physicalDevice;
	MemoryAllocator::logicalDevice = logicalDevice;

	VmaAllocatorCreateInfo info = {};
	info.physicalDevice = physicalDevice;
	info.device = logicalDevice;

	VkResult result = vmaCreateAllocator(&info, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create memory allocator.");
}

shared_ptr<Allocation> MemoryAllocator::createBuffer(const VkBufferCreateInfo& bufferInfo,
						     VmaMemoryUsage usage, VkBuffer& buffer)
{
	VmaAllocationCreateInfo createInfo = {};
	createInfo.usage = usage;

	if (usage != VMA_MEMORY_USAGE_GPU_ONLY) createInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VmaAllocation allocation = VK_NULL_HANDLE;
	VmaAllocationInfo allocationInfo = {};
	VkResult result = vmaCreateBuffer(handle, &bufferInfo, &createInfo, &buffer, &allocation,
					  &allocationInfo);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to allocate buffer memory.");

	return make_shared<Allocation>(logicalDevice, handle, allocation, allocationInfo);
}

shared_ptr<Allocation> MemoryAllocator::createImage(const VkImageCreateInfo& bufferInfo,
						    VmaMemoryUsage usage, VkImage& image)
{
	VmaAllocationCreateInfo createInfo = {};
	createInfo.usage = usage;

	if (usage != VMA_MEMORY_USAGE_GPU_ONLY) createInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VmaAllocation allocation = VK_NULL_HANDLE;
	VmaAllocationInfo allocationInfo = {};
	VkResult result = vmaCreateImage(handle, &bufferInfo, &createInfo, &image, &allocation,
					 &allocationInfo);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to allocate image memory.");

	return make_shared<Allocation>(logicalDevice, handle, allocation, allocationInfo);
}

}