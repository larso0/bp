#include <bp/Buffer.h>
#include <stdexcept>
#include <bp/Util.h>
#include <algorithm>

using namespace std;

namespace bp
{

void Buffer::init(NotNull<Device> device, VkDeviceSize size, VkBufferUsageFlags usage,
		  VkMemoryPropertyFlags requiredMemoryProperties,
		  VkMemoryPropertyFlags optimalMemoryProperties)
{
	if (isReady()) throw runtime_error("Buffer already initialized.");
	this->device = device;

	if (!(requiredMemoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
		usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.size = size;
	info.usage = usage;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateBuffer(*device, &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create buffer.");

	VkMemoryRequirements memoryRequirements = {};
	vkGetBufferMemoryRequirements(*device, handle, &memoryRequirements);


	int32_t memType = -1;
	if (optimalMemoryProperties != 0)
	{
		memType = findPhysicalDeviceMemoryType(*device,
						       memoryRequirements.memoryTypeBits,
						       optimalMemoryProperties);
		memoryProperties = optimalMemoryProperties;
	}
	if (memType == -1)
	{
		memType = findPhysicalDeviceMemoryType(*device,
						       memoryRequirements.memoryTypeBits,
						       requiredMemoryProperties);
		memoryProperties = requiredMemoryProperties;
	}
	if (memType == -1)
	{
		vkDestroyBuffer(*device, handle, nullptr);
		handle = VK_NULL_HANDLE;
		throw runtime_error("No suitable memory type.");
	}

	VkMemoryAllocateInfo memInfo = {};
	memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memInfo.allocationSize = memoryRequirements.size;
	memInfo.memoryTypeIndex = (uint32_t) memType;

	result = vkAllocateMemory(*device, &memInfo, nullptr, &memory);
	if (result != VK_SUCCESS)
	{
		vkDestroyBuffer(*device, handle, nullptr);
		handle = VK_NULL_HANDLE;
		throw runtime_error("Failed to allocate buffer memory.");
	}

	result = vkBindBufferMemory(*device, handle, memory, 0);
	if (result != VK_SUCCESS)
	{
		vkFreeMemory(*device, memory, nullptr);
		vkDestroyBuffer(*device, handle, nullptr);
		handle = VK_NULL_HANDLE;
		throw runtime_error("Failed to bind buffer memory.");
	}

	mapped.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mapped.pNext = nullptr;
	mapped.memory = memory;
	this->size = memoryRequirements.size;

	cmdPool.init(device->getTransferQueue(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
}

Buffer::~Buffer()
{
	if (!isReady()) return;
	delete stagingBuffer;
	vkFreeMemory(*device, memory, nullptr);
	vkDestroyBuffer(*device, handle, nullptr);
}

void* Buffer::map(VkDeviceSize offset, VkDeviceSize size)
{
	assertReady();
	void* mappedMemory;
	if (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		VkResult result = vkMapMemory(*device, memory, offset, size, 0, &mappedMemory);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to map buffer memory.");
	} else
	{
		if (stagingBuffer == nullptr)
		{
			stagingBuffer = new Buffer(device, this->size,
						   VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
						   VK_BUFFER_USAGE_TRANSFER_DST_BIT,
						   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}
		mappedMemory = stagingBuffer->map(offset, size);
	}

	mapped.offset = offset;
	mapped.size = size;
	return mappedMemory;
}

void Buffer::unmap(bool writeBack, VkCommandBuffer cmdBuffer)
{
	assertReady();
	if (stagingBuffer != nullptr)
	{
		stagingBuffer->unmap();
		if (writeBack)
			transfer(*stagingBuffer, mapped.offset, mapped.offset, mapped.size,
				 cmdBuffer);
	} else
	{
		vkFlushMappedMemoryRanges(*device, 1, &mapped);
		vkUnmapMemory(*device, memory);
	}
}

void Buffer::updateStagingBuffer(VkCommandBuffer cmdBuffer)
{
	if (memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) return;
	if (stagingBuffer == nullptr)
	{
		stagingBuffer = new Buffer(device, size,
					   VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
					   VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}
	stagingBuffer->transfer(*this, 0, 0, size, cmdBuffer);
}

void Buffer::transfer(VkDeviceSize offset, VkDeviceSize size, const void* data,
		      VkCommandBuffer cmdBuffer)
{
	assertReady();
	if (size == VK_WHOLE_SIZE) size = this->size - offset;
	if (stagingBuffer != nullptr)
	{
		void* mapped = stagingBuffer->map(offset, size);
		parallelCopy(mapped, data, size);
		stagingBuffer->unmap();

		bool useOwnBuffer = cmdBuffer == VK_NULL_HANDLE;
		if (useOwnBuffer)
			cmdBuffer = beginSingleUseCmdBuffer(*device, cmdPool);

		transfer(*stagingBuffer, offset, offset, size, cmdBuffer);

		if (useOwnBuffer)
			endSingleUseCmdBuffer(*device, *device->getTransferQueue(), cmdPool,
					      cmdBuffer);
	} else
	{
		void* mapped = map(offset, size);
		parallelCopy(mapped, data, size);
		unmap();
	}
}

void Buffer::transfer(Buffer& src, VkDeviceSize srcOffset, VkDeviceSize dstOffset,
		      VkDeviceSize size, VkCommandBuffer cmdBuffer)
{
	assertReady();
	bool useOwnBuffer = cmdBuffer == VK_NULL_HANDLE;
	if (useOwnBuffer)
		cmdBuffer = beginSingleUseCmdBuffer(*device, cmdPool);

	VkBufferCopy copy_region = {};
	copy_region.srcOffset = srcOffset;
	copy_region.dstOffset = dstOffset;

	if (size == VK_WHOLE_SIZE)
		copy_region.size = min(this->size - dstOffset, src.size - srcOffset);
	else
		copy_region.size = size;

	vkCmdCopyBuffer(cmdBuffer, src.getHandle(), handle, 1, &copy_region);

	if (useOwnBuffer)
		endSingleUseCmdBuffer(*device, *device->getTransferQueue(), cmdPool, cmdBuffer);
}

void Buffer::transfer(Image& src, VkCommandBuffer cmdBuffer)
{
	assertReady();
	bool useOwnBuffer = cmdBuffer == VK_NULL_HANDLE;
	if (useOwnBuffer)
		cmdBuffer = beginSingleUseCmdBuffer(*device, cmdPool);

	src.transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT,
		       VK_PIPELINE_STAGE_TRANSFER_BIT, cmdBuffer);

	VkImageSubresourceLayers subResource = {};
	subResource.aspectMask = src.format == VK_FORMAT_D16_UNORM ? VK_IMAGE_ASPECT_DEPTH_BIT
								   : VK_IMAGE_ASPECT_COLOR_BIT;
	subResource.baseArrayLayer = 0;
	subResource.mipLevel = 0;
	subResource.layerCount = 1;

	VkBufferImageCopy region = {};
	region.imageSubresource = subResource;
	region.imageExtent = {src.width, src.height, 1};

	vkCmdCopyImageToBuffer(cmdBuffer, src, src.layout, handle, 1, &region);

	if (useOwnBuffer)
		endSingleUseCmdBuffer(*device, *device->getTransferQueue(), cmdPool, cmdBuffer);
}

void Buffer::assertReady()
{
	if (!isReady())
		throw runtime_error("Buffer not ready. Must initialize before use.");
}

}