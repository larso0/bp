#include <bp/Image.h>
#include <stdexcept>
#include <bp/Util.h>

using namespace std;

namespace bp
{

Image::~Image()
{
	if (isReady())
	{
		vkFreeMemory(device, memory, nullptr);
		vkDestroyImage(device, handle, nullptr);
	}
}

void Image::init()
{
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

	VkMemoryRequirements requirements;
	vkGetImageMemoryRequirements(device, handle, &requirements);

	int32_t memType = findPhysicalDeviceMemoryType(physicalDevice, requirements.memoryTypeBits,
						       memoryProperties);
	if (memType == -1)
		throw runtime_error("No suitable memory type.");

	VkMemoryAllocateInfo memInfo = {};
	memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memInfo.allocationSize = requirements.size;
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
	memorySize = requirements.size;
}

void* Image::map(VkDeviceSize offset, VkDeviceSize size)
{
	void* mappedMemory;
	VkResult result = vkMapMemory(device, memory, offset, size, 0, &mappedMemory);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to map image memory.");
	mapped.offset = offset;
	mapped.size = size;
	return mappedMemory;
}

void Image::unmap()
{
	vkFlushMappedMemoryRanges(device, 1, &mapped);
	vkUnmapMemory(device, memory);
}

void Image::transition(VkImageLayout dstLayout, VkAccessFlags dstAccess,
		       VkPipelineStageFlags dstStage, VkCommandBuffer cmdBuffer)
{
	if (dstLayout == layout) return;

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

	layout = dstLayout;
	accessFlags = dstAccess;
}

void Image::transfer(Image& fromImage, VkCommandBuffer cmdBuffer)
{
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
}

void Image::setDevice(VkPhysicalDevice physical, VkDevice logical)
{
	if (isReady())
		throw runtime_error("Failed to alter device, image already created.");
	physicalDevice = physical;
	device = logical;
}

void Image::setSize(uint32_t w, uint32_t h)
{
	if (isReady())
		throw runtime_error("Failed to alter size, image already created.");
	width = w;
	height = h;
}

void Image::setFormat(VkFormat format)
{
	if (isReady())
		throw runtime_error("Failed to alter format, image already created.");
	this->format = format;
}

void Image::setTiling(VkImageTiling tiling)
{
	if (isReady())
		throw runtime_error("Failed to alter tiling, image already created.");
	this->tiling = tiling;
}

void Image::setLayout(VkImageLayout layout)
{
	if (isReady())
		throw runtime_error("Failed to alter layout, image already created.");
	this->layout = layout;
}

void Image::setAccessFlags(VkAccessFlags flags)
{
	if (isReady())
		throw runtime_error("Failed to alter access flags, image already created.");
	accessFlags = flags;
}

void Image::setUsage(VkImageUsageFlags usage)
{
	if (isReady())
		throw runtime_error("Failed to alter usage, image already created.");
	this->usage = usage;
}

void Image::setMemoryProperties(VkMemoryPropertyFlags properties)
{
	if (isReady())
		throw runtime_error("Failed to alter memory properties, image already created.");
	memoryProperties = properties;
}

}