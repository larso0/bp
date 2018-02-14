#include <bp/MemoryAllocator.h>
#include <bp/Allocation.h>
#include <bp/ExternalMemory.h>
#include <bp/MemoryAllocator.h>
#include <bp/Device.h>
#include <stdexcept>

using namespace std;

namespace bp
{

MemoryAllocator::~MemoryAllocator()
{
	if (isReady())
		vmaDestroyAllocator(handle);
}

void MemoryAllocator::init(Device& device)
{
	physicalDevice = device;
	logicalDevice = device;

	VmaAllocatorCreateInfo info = {};
	info.physicalDevice = physicalDevice;
	info.device = logicalDevice;

	VkResult result = vmaCreateAllocator(&info, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create memory allocator.");
}

shared_ptr<Memory> MemoryAllocator::createBuffer(const VkBufferCreateInfo& bufferInfo,
						 VmaMemoryUsage usage, VkBuffer& buffer,
						 void* opaque)
{
	if (opaque == nullptr)
	{
		VmaAllocationCreateInfo createInfo = {};
		createInfo.usage = usage;

		if (usage != VMA_MEMORY_USAGE_GPU_ONLY)
			createInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocation allocation = VK_NULL_HANDLE;
		VmaAllocationInfo allocationInfo = {};
		VkResult result = vmaCreateBuffer(handle, &bufferInfo, &createInfo, &buffer,
						  &allocation,
						  &allocationInfo);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create buffer.");

		return make_shared<Allocation>(logicalDevice, handle, allocation, allocationInfo);
	} else
	{
		VkResult result = vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create buffer.");

		VkMemoryRequirements requirements = {};
		vkGetBufferMemoryRequirements(logicalDevice, buffer, &requirements);

		return make_shared<ExternalMemory>(physicalDevice, logicalDevice, requirements,
						   opaque);
	}
}

shared_ptr<Memory> MemoryAllocator::createImage(const VkImageCreateInfo& imageInfo,
						VmaMemoryUsage usage, VkImage& image,
						void* opaque)
{
	if (opaque == nullptr)
	{
		VmaAllocationCreateInfo createInfo = {};
		createInfo.usage = usage;

		if (usage != VMA_MEMORY_USAGE_GPU_ONLY)
			createInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocation allocation = VK_NULL_HANDLE;
		VmaAllocationInfo allocationInfo = {};
		VkResult result = vmaCreateImage(handle, &imageInfo, &createInfo, &image,
						 &allocation,
						 &allocationInfo);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create image.");

		return make_shared<Allocation>(logicalDevice, handle, allocation, allocationInfo);
	} else
	{
		VkResult result = vkCreateImage(logicalDevice, &imageInfo, nullptr, &image);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create image.");

		VkMemoryRequirements requirements = {};
		vkGetImageMemoryRequirements(logicalDevice, image, &requirements);

		return make_shared<ExternalMemory>(physicalDevice, logicalDevice, requirements,
						   opaque);
	}
}

}