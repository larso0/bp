#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 1
#include <bp/MemoryAllocator.h>
#include <stdexcept>

using namespace std;

namespace bp
{

Allocation::Allocation(Allocation&& other)
{
	if (isReady()) vmaFreeMemory(allocator, handle);
	device = other.device;
	allocator = other.allocator;
	handle = other.handle;
	info = other.info;
	coherent = other.coherent;
	other.device = VK_NULL_HANDLE;
	other.allocator = VMA_NULL;
	other.handle = VMA_NULL;
	other.info = {};
}

Allocation::~Allocation()
{
	if (isReady()) vmaFreeMemory(allocator, handle);
}

void Allocation::flushMapped()
{
	if (coherent) return;
	VkMappedMemoryRange range = {};
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.memory = info.deviceMemory;
	range.offset = info.offset;
	range.size = info.size;
	vkFlushMappedMemoryRanges(device, 1, &range);
}

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

	VmaAllocation allocation = VMA_NULL;
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

	VmaAllocation allocation = VMA_NULL;
	VmaAllocationInfo allocationInfo = {};
	VkResult result = vmaCreateImage(handle, &bufferInfo, &createInfo, &image, &allocation,
					 &allocationInfo);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to allocate image memory.");

	return make_shared<Allocation>(logicalDevice, handle, allocation, allocationInfo);
}

}