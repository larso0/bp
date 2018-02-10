#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 1
#include <bp/Allocation.h>

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
	other.allocator = VK_NULL_HANDLE;
	other.handle = VK_NULL_HANDLE;
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

}