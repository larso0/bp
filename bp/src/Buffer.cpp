#include <bp/Buffer.h>
#include <stdexcept>
#include <bp/Util.h>
#include <cstring>
#include <algorithm>

using namespace std;

namespace bp
{

Buffer::~Buffer()
{
	if (isReady())
	{
		if (cmdPool != VK_NULL_HANDLE)
			vkDestroyCommandPool(device, cmdPool, nullptr);
		if (stagingBuffer != nullptr)
			delete stagingBuffer;
		vkFreeMemory(device, memory, nullptr);
		vkDestroyBuffer(device, handle, nullptr);
	}
}

void Buffer::init()
{
	if (!(requiredMemoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
		usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.size = size;
	info.usage = usage;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(device, &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create buffer.");

	VkMemoryRequirements memoryRequirements = {};
	vkGetBufferMemoryRequirements(device, handle, &memoryRequirements);


	int32_t memType = -1;
	if (optimalMemoryProperties != 0)
	{
		memType = findPhysicalDeviceMemoryType(physicalDevice,
						       memoryRequirements.memoryTypeBits,
						       optimalMemoryProperties);
		memoryProperties = optimalMemoryProperties;
	}
	if (memType == -1)
	{
		memType = findPhysicalDeviceMemoryType(physicalDevice,
						       memoryRequirements.memoryTypeBits,
						       requiredMemoryProperties);
		memoryProperties = requiredMemoryProperties;
	}
	if (memType == -1)
		throw runtime_error("No suitable memory type.");

	VkMemoryAllocateInfo memInfo = {};
	memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memInfo.allocationSize = memoryRequirements.size;
	memInfo.memoryTypeIndex = (uint32_t) memType;

	result = vkAllocateMemory(device, &memInfo, nullptr, &memory);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to allocate buffer memory.");

	result = vkBindBufferMemory(device, handle, memory, 0);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to bind buffer memory.");

	mapped.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mapped.pNext = nullptr;
	mapped.memory = memory;
}

void* Buffer::map(VkDeviceSize offset, VkDeviceSize size)
{
	void* mappedMemory;
	if (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		VkResult result = vkMapMemory(device, memory, offset, size, 0, &mappedMemory);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to map buffer memory.");
	} else if (transferQueue != VK_NULL_HANDLE)
	{
		if (stagingBuffer == nullptr)
		{
			stagingBuffer = new Buffer();
			stagingBuffer->setDevice(physicalDevice, device);
			stagingBuffer->setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			stagingBuffer->setRequiredMemoryProperties(
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagingBuffer->setSize(this->size);
			stagingBuffer->init();
		}
		mappedMemory = stagingBuffer->map(offset, size);
	} else
		throw runtime_error("Buffer is not mappable.");

	mapped.offset = offset;
	mapped.size = size;
	return mappedMemory;
}

void Buffer::unmap()
{
	if (stagingBuffer != nullptr)
	{
		stagingBuffer->unmap();
		transfer(*stagingBuffer, mapped.offset, mapped.offset, mapped.size);
	} else
	{
		vkFlushMappedMemoryRanges(device, 1, &mapped);
		vkUnmapMemory(device, memory);
	}
}

void Buffer::transfer(VkDeviceSize offset, VkDeviceSize size, const void* data,
		      VkCommandBuffer cmdBuffer)
{
	if (size == VK_WHOLE_SIZE) size = this->size - offset;
	if (stagingBuffer != nullptr)
	{
		void* mapped = stagingBuffer->map(offset, size);
		memcpy(mapped, data, size);
		stagingBuffer->unmap();

		bool useOwnBuffer = cmdBuffer == VK_NULL_HANDLE;
		if (useOwnBuffer)
		{
			if (cmdPool == VK_NULL_HANDLE) createCommandPool();
			cmdBuffer = beginSingleUseCmdBuffer(device, cmdPool);
		}

		transfer(*stagingBuffer, offset, offset, size, cmdBuffer);

		if (useOwnBuffer)
			endSingleUseCmdBuffer(device, transferQueue, cmdPool, cmdBuffer);
	} else
	{
		void* mapped = map(offset, size);
		memcpy(mapped, data, size);
		unmap();
	}
}

void Buffer::transfer(Buffer& src, VkDeviceSize srcOffset, VkDeviceSize dstOffset,
		      VkDeviceSize size, VkCommandBuffer cmdBuffer)
{
	bool useOwnBuffer = cmdBuffer == VK_NULL_HANDLE;
	if (useOwnBuffer)
	{
		if (cmdPool == VK_NULL_HANDLE) createCommandPool();
		cmdBuffer = beginSingleUseCmdBuffer(device, cmdPool);
	}

	VkBufferCopy copy_region = {};
	copy_region.srcOffset = srcOffset;
	copy_region.dstOffset = dstOffset;

	if (size == VK_WHOLE_SIZE)
		copy_region.size = min(this->size - dstOffset, src.size - srcOffset);
	else
		copy_region.size = size;

	vkCmdCopyBuffer(cmdBuffer, src.getHandle(), handle, 1, &copy_region);

	if (useOwnBuffer)
		endSingleUseCmdBuffer(device, transferQueue, cmdPool, cmdBuffer);
}

void Buffer::setDevice(VkPhysicalDevice physical, VkDevice logical)
{
	if (isReady())
		throw runtime_error("Failed to alter device, buffer already created.");
	physicalDevice = physical;
	device = logical;
}

void Buffer::setTransferQueue(uint32_t queueFamilyIndex, VkQueue queue)
{
	if (isReady() && cmdPool != VK_NULL_HANDLE)
	{
		vkDestroyCommandPool(device, cmdPool, nullptr);
		cmdPool = VK_NULL_HANDLE;
	}
	transferQueueFamilyIndex = queueFamilyIndex;
	transferQueue = queue;
}

void Buffer::setUsage(VkBufferUsageFlags usage)
{
	if (isReady())
		throw runtime_error("Failed to alter usage, buffer already created.");
	this->usage = usage;
}

void Buffer::setRequiredMemoryProperties(VkMemoryPropertyFlags properties)
{
	requiredMemoryProperties = properties;
}

void Buffer::setOptimalMemoryProperties(VkMemoryPropertyFlags properties)
{
	optimalMemoryProperties = properties;
}

void Buffer::setSize(VkDeviceSize size)
{
	if (isReady())
		throw runtime_error("Failed to alter size, buffer already created.");
	this->size = size;
}

void Buffer::createCommandPool()
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolInfo.queueFamilyIndex = transferQueueFamilyIndex;

	VkResult result = vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create command pool.");
}

}