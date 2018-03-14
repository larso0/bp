#ifndef BP_IMAGE_H
#define BP_IMAGE_H

#include "Device.h"
#include "CommandPool.h"

namespace bp
{

class Buffer;

class Image
{
public:
	Image() :
		device{nullptr},
		handle{VK_NULL_HANDLE},
		width{0}, height{0},
		format{VK_FORMAT_UNDEFINED},
		tiling{VK_IMAGE_TILING_LINEAR},
		usage{0},
		accessFlags{0},
		memory{VK_NULL_HANDLE},
		stagingBuffer{nullptr} {}
	Image(Device& device, uint32_t width, uint32_t height, VkFormat format,
	      VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage,
	      VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED) :
		Image()
	{
		init(device, width, height, format, tiling, usage, memoryUsage, initialLayout);
	}
	~Image();

	void init(Device& device, uint32_t width, uint32_t height, VkFormat format,
		  VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memoryUsage,
		  VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);

	void* map();
	void createStagingBuffer();
	void freeStagingBuffer();
	void updateStagingBuffer(VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);
	void flushStagingBuffer(VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);

	void transition(VkImageLayout dstLayout, VkAccessFlags dstAccess,
			VkPipelineStageFlags dstStage, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);
	void transfer(Image& fromImage, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);
	void transfer(Buffer& src, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);

	operator VkImage() { return handle; }

	VkImage getHandle() { return handle; }
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	VkFormat getFormat() const { return format; }
	VkImageTiling getTiling() const { return tiling; }
	VkImageUsageFlags getUsage() const { return usage; }
	VkImageLayout getLayout() const { return layout; }
	VkDeviceSize getMemorySize() const { return memory->getSize(); }
	Buffer* getStagingBuffer() { return stagingBuffer; }
	bool isReady() const { return handle != VK_NULL_HANDLE; }
private:
	friend class Buffer;

	Device* device;
	VkImage handle;
	CommandPool cmdPool;
	uint32_t width, height;
	VkFormat format;
	VkImageTiling tiling;
	VkImageUsageFlags usage;
	VkImageLayout layout;
	VkAccessFlags accessFlags;

	std::shared_ptr<Memory> memory;
	Buffer* stagingBuffer;

	void assertReady();
};

}

#endif