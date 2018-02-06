#include <bp/Buffer.h>
#include <stdexcept>
#include <bp/Util.h>
#include <algorithm>

using namespace std;

namespace bp
{

void Buffer::init(Device& device, VkDeviceSize size, VkBufferUsageFlags usage,
		  VmaMemoryUsage memoryUsage)
{
	if (isReady()) throw runtime_error("Buffer already initialized.");
	Buffer::device = &device;
	Buffer::size = size;

	usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.size = size;
	info.usage = usage;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	memory = device.getMemoryAllocator().createBuffer(info, memoryUsage, handle);

	cmdPool.init(device.getTransferQueue(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
}

Buffer::~Buffer()
{
	if (!isReady()) return;
	delete stagingBuffer;
	vkDestroyBuffer(*device, handle, nullptr);
}

void* Buffer::map()
{
	assertReady();
	if (memory->isMapped()) return memory->getMapped();

	if (stagingBuffer == nullptr) createStagingBuffer();
	return stagingBuffer->map();
}

void Buffer::createStagingBuffer()
{
	if (stagingBuffer != nullptr)
		throw runtime_error("Staging buffer is already created.");
	stagingBuffer = new Buffer(*device, Buffer::size,
				   VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
				   VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				   VMA_MEMORY_USAGE_CPU_ONLY);
}

void Buffer::freeStagingBuffer()
{
	delete stagingBuffer;
	stagingBuffer = nullptr;
}

void Buffer::updateStagingBuffer(VkCommandBuffer cmdBuffer)
{
	if (memory->isMapped()) return;
	if (stagingBuffer == nullptr) createStagingBuffer();
	stagingBuffer->transfer(*this, 0, 0, size, cmdBuffer);
}

void Buffer::flushStagingBuffer(VkCommandBuffer cmdBuffer)
{
	if (stagingBuffer == nullptr) return;
	transfer(*stagingBuffer, 0, 0, size, cmdBuffer);
}

void Buffer::transfer(VkDeviceSize offset, VkDeviceSize size, const void* data,
		      VkCommandBuffer cmdBuffer)
{
	assertReady();
	if (size == VK_WHOLE_SIZE) size = Buffer::size - offset;
	void* mapped = map();
	parallelCopy(mapped, data, size);

	if (stagingBuffer != nullptr)
	{
		bool useOwnBuffer = cmdBuffer == VK_NULL_HANDLE;
		if (useOwnBuffer)
		{
			cmdBuffer = cmdPool.allocateCommandBuffer();
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkBeginCommandBuffer(cmdBuffer, &beginInfo);
		}

		transfer(*stagingBuffer, offset, offset, size, cmdBuffer);

		if (useOwnBuffer)
		{
			vkEndCommandBuffer(cmdBuffer);
			Queue& queue = device->getTransferQueue();
			queue.submit({}, {cmdBuffer}, {});
			queue.waitIdle();
			cmdPool.freeCommandBuffer(cmdBuffer);
		}
	}
}

void Buffer::transfer(Buffer& src, VkDeviceSize srcOffset, VkDeviceSize dstOffset,
		      VkDeviceSize size, VkCommandBuffer cmdBuffer)
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

	VkBufferCopy copy_region = {};
	copy_region.srcOffset = srcOffset;
	copy_region.dstOffset = dstOffset;

	if (size == VK_WHOLE_SIZE)
		copy_region.size = min(Buffer::size - dstOffset, src.size - srcOffset);
	else
		copy_region.size = size;

	vkCmdCopyBuffer(cmdBuffer, src.getHandle(), handle, 1, &copy_region);

	if (useOwnBuffer)
	{
		vkEndCommandBuffer(cmdBuffer);
		Queue& queue = device->getTransferQueue();
		queue.submit({}, {cmdBuffer}, {});
		queue.waitIdle();
		cmdPool.freeCommandBuffer(cmdBuffer);
	}
}

void Buffer::transfer(Image& src, VkCommandBuffer cmdBuffer)
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
	{
		vkEndCommandBuffer(cmdBuffer);
		Queue& queue = device->getTransferQueue();
		queue.submit({}, {cmdBuffer}, {});
		queue.waitIdle();
		cmdPool.freeCommandBuffer(cmdBuffer);
	}
}

void Buffer::assertReady()
{
	if (!isReady())
		throw runtime_error("Buffer not ready. Must initialize before use.");
}

}