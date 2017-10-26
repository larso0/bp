#include <bp/Image.h>
#include <stdexcept>
#include <bp/Util.h>

using namespace std;

namespace bp
{

Image::Image(Device& device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
	     VkImageUsageFlags usage, VkMemoryPropertyFlags requiredMemoryProperties,
	     VkMemoryPropertyFlags optimalMemoryProperties, VkImageLayout initialLayout) :
	device{device},
	cmdPool{VK_NULL_HANDLE},
	width{width}, height{height},
	format{format},
	tiling{tiling},
	layout{initialLayout},
	accessFlags{0},
	mapped{},
	stagingImage{0}
{
	if (!(requiredMemoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
		usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

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

	VkResult result = vkCreateImage(device, &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create image.");

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device, handle, &memoryRequirements);

	int32_t memType = -1;
	if (optimalMemoryProperties != 0)
	{
		memType = findPhysicalDeviceMemoryType(device,
						       memoryRequirements.memoryTypeBits,
						       optimalMemoryProperties);
		memoryProperties = optimalMemoryProperties;
	}
	if (memType == -1)
	{
		memType = findPhysicalDeviceMemoryType(device,
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

	result = vkAllocateMemory(device, &memInfo, nullptr,
				  &memory);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to allocate image memory.");

	result = vkBindImageMemory(device, handle, memory, 0);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to bind image memory.");

	mapped.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mapped.pNext = nullptr;
	mapped.memory = memory;
	memorySize = memoryRequirements.size;
}

Image::~Image()
{
	if (cmdPool != VK_NULL_HANDLE) vkDestroyCommandPool(device, cmdPool, nullptr);
	if (stagingImage != nullptr) delete stagingImage;
	vkFreeMemory(device, memory, nullptr);
	vkDestroyImage(device, handle, nullptr);
}

void* Image::map(VkDeviceSize offset, VkDeviceSize size)
{
	void* mappedMemory;
	if (tiling == VK_IMAGE_TILING_LINEAR &&
	    memoryProperties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
	{
		transition(VK_IMAGE_LAYOUT_GENERAL,
			   VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT,
			   VK_PIPELINE_STAGE_HOST_BIT);
		VkResult result = vkMapMemory(device, memory, offset, size, 0, &mappedMemory);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to map Image memory.");
	} else
	{
		if (stagingImage == nullptr)
		{
			stagingImage = new Image(device, width, height, format,
						 VK_IMAGE_TILING_LINEAR,
						 VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
						 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}
		mappedMemory = stagingImage->map(offset, size);
	}

	mapped.offset = offset;
	mapped.size = size;
	return mappedMemory;
}

void Image::unmap(bool writeBack)
{
	if (stagingImage != nullptr)
	{
		stagingImage->unmap();
		if (writeBack)
			transfer(*stagingImage);
	} else
	{
		vkFlushMappedMemoryRanges(device, 1, &mapped);
		vkUnmapMemory(device, memory);
	}
}

void Image::transition(VkImageLayout dstLayout, VkAccessFlags dstAccess,
		       VkPipelineStageFlags dstStage, VkCommandBuffer cmdBuffer)
{
	if (dstLayout == layout) return;

	bool useOwnBuffer = cmdBuffer == VK_NULL_HANDLE;
	if (useOwnBuffer)
	{
		if (cmdPool == VK_NULL_HANDLE) createCommandPool();
		cmdBuffer = beginSingleUseCmdBuffer(device, cmdPool);
	}

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

	if (dstLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, dstStage, 0, 0, nullptr,
			     0, nullptr, 1, &barrier);

	if (useOwnBuffer)
		endSingleUseCmdBuffer(device, device.getGraphicsQueue(), cmdPool, cmdBuffer);

	layout = dstLayout;
	accessFlags = dstAccess;
}

void Image::transfer(Image& fromImage, VkCommandBuffer cmdBuffer)
{
	bool useOwnBuffer = cmdBuffer == VK_NULL_HANDLE;
	if (useOwnBuffer)
	{
		if (cmdPool == VK_NULL_HANDLE) createCommandPool();
		cmdBuffer = beginSingleUseCmdBuffer(device, cmdPool);
	}

	fromImage.transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT,
			     VK_PIPELINE_STAGE_TRANSFER_BIT, cmdBuffer);
	transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
		   VK_PIPELINE_STAGE_TRANSFER_BIT, cmdBuffer);

	VkImageSubresourceLayers subResource = {};
	subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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
		endSingleUseCmdBuffer(device, device.getGraphicsQueue(), cmdPool, cmdBuffer);
}

void Image::createCommandPool()
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolInfo.queueFamilyIndex = device.getGraphicsQueue().getQueueFamilyIndex();

	VkResult result = vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create command pool.");
}

}