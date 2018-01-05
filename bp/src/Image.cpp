#include <bp/Image.h>
#include <bp/Buffer.h>
#include <stdexcept>
#include <bp/Util.h>

using namespace std;

namespace bp
{

void Image::init(NotNull<Device> device, uint32_t width, uint32_t height, VkFormat format,
		 VkImageTiling tiling, VkImageUsageFlags usage,
		 VkMemoryPropertyFlags requiredMemoryProperties,
		 VkMemoryPropertyFlags optimalMemoryProperties, VkImageLayout initialLayout)
{
	if (isReady()) throw runtime_error("Image already initialized.");
	this->device = device;
	this->width = width;
	this->height = height;
	this->format = format;
	this->tiling = tiling;
	this->usage = usage;
	this->layout = initialLayout;

	if (!(requiredMemoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
		usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	VkImageCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.imageType = VK_IMAGE_TYPE_2D;
	info.extent.width = width;
	info.extent.height = height;
	info.extent.depth = 1;
	info.mipLevels = 1;
	info.arrayLayers = 1;
	info.format = format;
	info.tiling = tiling;
	info.initialLayout = layout;
	info.usage = usage;
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkResult result = vkCreateImage(*device, &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create image.");

	VkMemoryRequirements memoryRequirements = {};
	vkGetImageMemoryRequirements(*device, handle, &memoryRequirements);

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
		vkDestroyImage(*device, handle, nullptr);
		handle = VK_NULL_HANDLE;
		throw runtime_error("No suitable memory type.");
	}

	VkMemoryAllocateInfo memInfo = {};
	memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memInfo.allocationSize = memoryRequirements.size;
	memInfo.memoryTypeIndex = (uint32_t) memType;

	result = vkAllocateMemory(*device, &memInfo, nullptr,
				  &memory);
	if (result != VK_SUCCESS)
	{
		vkDestroyImage(*device, handle, nullptr);
		handle = VK_NULL_HANDLE;
		throw runtime_error("Failed to allocate image memory.");
	}

	result = vkBindImageMemory(*device, handle, memory, 0);
	if (result != VK_SUCCESS)
	{
		vkFreeMemory(*device, memory, nullptr);
		vkDestroyImage(*device, handle, nullptr);
		handle = VK_NULL_HANDLE;
		throw runtime_error("Failed to bind image memory.");
	}

	mapped.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mapped.pNext = nullptr;
	mapped.memory = memory;
	memorySize = memoryRequirements.size;

	cmdPool.init(device->getTransferQueue(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
}

Image::~Image()
{
	if (!isReady()) return;
	delete stagingBuffer;
	vkFreeMemory(*device, memory, nullptr);
	vkDestroyImage(*device, handle, nullptr);
}

void* Image::map(VkDeviceSize offset, VkDeviceSize size)
{
	assertReady();
	void* mappedMemory;
	if (tiling == VK_IMAGE_TILING_LINEAR &&
	    memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		transition(VK_IMAGE_LAYOUT_GENERAL,
			   VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT,
			   VK_PIPELINE_STAGE_HOST_BIT);
		VkResult result = vkMapMemory(*device, memory, offset, size, 0, &mappedMemory);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to map Image memory.");
	} else
	{
		if (stagingBuffer == nullptr)
		{
			stagingBuffer = new Buffer(device, memorySize,
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

void Image::unmap(bool writeBack, VkCommandBuffer cmdBuffer)
{
	assertReady();
	if (stagingBuffer != nullptr)
	{
		stagingBuffer->unmap();
		if (writeBack)
			transfer(*stagingBuffer, cmdBuffer);
	} else
	{
		vkFlushMappedMemoryRanges(*device, 1, &mapped);
		vkUnmapMemory(*device, memory);
	}
}

void Image::transition(VkImageLayout dstLayout, VkAccessFlags dstAccess,
		       VkPipelineStageFlags dstStage, VkCommandBuffer cmdBuffer)
{
	assertReady();
	if (dstLayout == layout) return;

	bool useOwnBuffer = cmdBuffer == VK_NULL_HANDLE;
	if (useOwnBuffer)
		cmdBuffer = beginSingleUseCmdBuffer(*device, cmdPool);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = layout;
	barrier.newLayout = dstLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = handle;
	barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
	barrier.srcAccessMask = accessFlags;
	barrier.dstAccessMask = dstAccess;

	if (format == VK_FORMAT_D16_UNORM)
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, dstStage, 0, 0, nullptr,
			     0, nullptr, 1, &barrier);

	if (useOwnBuffer)
		endSingleUseCmdBuffer(*device, *device->getTransferQueue(), cmdPool, cmdBuffer);

	layout = dstLayout;
	accessFlags = dstAccess;
}

void Image::transfer(Image& fromImage, VkCommandBuffer cmdBuffer)
{
	assertReady();
	bool useOwnBuffer = cmdBuffer == VK_NULL_HANDLE;
	if (useOwnBuffer)
		cmdBuffer = beginSingleUseCmdBuffer(*device, cmdPool);

	fromImage.transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT,
			     VK_PIPELINE_STAGE_TRANSFER_BIT, cmdBuffer);
	transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
		   VK_PIPELINE_STAGE_TRANSFER_BIT, cmdBuffer);

	VkImageSubresourceLayers subResource = {};
	subResource.aspectMask = format == VK_FORMAT_D16_UNORM ?
				    VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	subResource.baseArrayLayer = 0;
	subResource.mipLevel = 0;
	subResource.layerCount = 1;

	VkImageCopy region = {};
	region.srcSubresource = subResource;
	region.dstSubresource = subResource;
	region.srcOffset = {0, 0, 0};
	region.dstOffset = {0, 0, 0};
	region.extent.width = width;
	region.extent.height = height;
	region.extent.depth = 1;

	vkCmdCopyImage(cmdBuffer, fromImage.getHandle(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		       handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	if (useOwnBuffer)
		endSingleUseCmdBuffer(*device, *device->getTransferQueue(), cmdPool, cmdBuffer);
}

void Image::transfer(Buffer& src, VkCommandBuffer cmdBuffer)
{
	assertReady();
	bool useOwnBuffer = cmdBuffer == VK_NULL_HANDLE;
	if (useOwnBuffer)
		cmdBuffer = beginSingleUseCmdBuffer(*device, cmdPool);

	transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
		   VK_PIPELINE_STAGE_TRANSFER_BIT, cmdBuffer);

	VkImageSubresourceLayers subResource = {};
	subResource.aspectMask = format == VK_FORMAT_D16_UNORM ? VK_IMAGE_ASPECT_DEPTH_BIT
							       : VK_IMAGE_ASPECT_COLOR_BIT;
	subResource.baseArrayLayer = 0;
	subResource.mipLevel = 0;
	subResource.layerCount = 1;

	VkBufferImageCopy region = {};
	region.imageSubresource = subResource;
	region.imageExtent = {width, height, 1};

	vkCmdCopyBufferToImage(cmdBuffer, src, handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
			       &region);

	if (useOwnBuffer)
		endSingleUseCmdBuffer(*device, *device->getTransferQueue(), cmdPool, cmdBuffer);
}

void Image::assertReady()
{
	if (!isReady()) throw runtime_error("Image not ready. Must be initialized before use.");
}

}