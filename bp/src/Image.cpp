#include <bp/Image.h>
#include <bp/Buffer.h>
#include <stdexcept>
#include <bp/Util.h>

using namespace std;

namespace bp
{

void Image::init(Device& device, uint32_t width, uint32_t height, VkFormat format,
		 VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage,
		 VkImageLayout initialLayout)
{
	if (isReady()) throw runtime_error("Image already initialized.");

	usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	Image::device = &device;
	Image::width = width;
	Image::height = height;
	Image::format = format;
	Image::tiling = tiling;
	Image::usage = usage;
	Image::layout = initialLayout;

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

	memory = device.getMemoryAllocator().createImage(info, memoryUsage, handle);

	cmdPool.init(device.getTransferQueue(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
}

Image::~Image()
{
	if (!isReady()) return;
	delete stagingBuffer;
	vkDestroyImage(*device, handle, nullptr);
}

void* Image::map()
{
	assertReady();
	if (tiling == VK_IMAGE_TILING_LINEAR && memory->isMapped()) return memory->getMapped();

	if (stagingBuffer == nullptr) createStagingBuffer();
	return stagingBuffer->map();
}

void Image::createStagingBuffer()
{
	if (stagingBuffer != nullptr)
		throw runtime_error("Staging buffer is already created.");
	stagingBuffer = new Buffer(*device, getMemorySize(),
				   VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
				   VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				   VMA_MEMORY_USAGE_CPU_ONLY);
}

void Image::freeStagingBuffer()
{
	delete stagingBuffer;
	stagingBuffer = nullptr;
}

void Image::updateStagingBuffer(VkCommandBuffer cmdBuffer)
{
	if (tiling == VK_IMAGE_TILING_LINEAR && memory->isMapped()) return;
	if (stagingBuffer == nullptr) createStagingBuffer();
	stagingBuffer->transfer(*this, cmdBuffer);
}

void Image::flushStagingBuffer(VkCommandBuffer cmdBuffer)
{
	if (stagingBuffer == nullptr) return;
	transfer(*stagingBuffer, cmdBuffer);
}

void Image::transition(VkImageLayout dstLayout, VkAccessFlags dstAccess,
		       VkPipelineStageFlags dstStage, VkCommandBuffer cmdBuffer)
{
	assertReady();
	if (dstLayout == layout) return;

	bool useOwnBuffer = cmdBuffer == VK_NULL_HANDLE;
	if (useOwnBuffer)
	{
		cmdBuffer = cmdPool.allocateCommandBuffer();
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cmdBuffer, &beginInfo);
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

	if (format == VK_FORMAT_D16_UNORM)
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

	vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, dstStage, 0, 0, nullptr,
			     0, nullptr, 1, &barrier);

	if (useOwnBuffer)
	{
		vkEndCommandBuffer(cmdBuffer);
		Queue& queue = device->getTransferQueue();
		queue.submit({}, {cmdBuffer}, {});
		queue.waitIdle();
		cmdPool.freeCommandBuffer(cmdBuffer);
	}

	layout = dstLayout;
	accessFlags = dstAccess;
}

void Image::transfer(Image& fromImage, VkCommandBuffer cmdBuffer)
{
	assertReady();
	bool useOwnBuffer = cmdBuffer == VK_NULL_HANDLE;
	if (useOwnBuffer)
	{
		cmdBuffer = cmdPool.allocateCommandBuffer();
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cmdBuffer, &beginInfo);
	}

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
	{
		vkEndCommandBuffer(cmdBuffer);
		Queue& queue = device->getTransferQueue();
		queue.submit({}, {cmdBuffer}, {});
		queue.waitIdle();
		cmdPool.freeCommandBuffer(cmdBuffer);
	}
}

void Image::transfer(Buffer& src, VkCommandBuffer cmdBuffer)
{
	assertReady();
	bool useOwnBuffer = cmdBuffer == VK_NULL_HANDLE;
	if (useOwnBuffer)
	{
		cmdBuffer = cmdPool.allocateCommandBuffer();
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(cmdBuffer, &beginInfo);
	}

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
	{
		vkEndCommandBuffer(cmdBuffer);
		Queue& queue = device->getTransferQueue();
		queue.submit({}, {cmdBuffer}, {});
		queue.waitIdle();
		cmdPool.freeCommandBuffer(cmdBuffer);
	}
}

void Image::assertReady()
{
	if (!isReady()) throw runtime_error("Image not ready. Must be initialized before use.");
}

}