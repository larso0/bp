#ifndef BP_IMAGE_H
#define BP_IMAGE_H

#include "Device.h"
#include "Pointer.h"

namespace bp
{

class Buffer;

class Image
{
public:
	Image() :
		device{nullptr},
		handle{VK_NULL_HANDLE},
		cmdPool{VK_NULL_HANDLE},
		width{0}, height{0},
		format{VK_FORMAT_UNDEFINED},
		tiling{VK_IMAGE_TILING_LINEAR},
		usage{0},
		accessFlags{0},
		memoryProperties{0},
		memorySize{0},
		memory{VK_NULL_HANDLE},
		mapped{},
		stagingBuffer{nullptr} {}
	Image(NotNull<Device> device, uint32_t width, uint32_t height, VkFormat format,
	      VkImageTiling tiling, VkImageUsageFlags usage,
	      VkMemoryPropertyFlags requiredMemoryProperties,
	      VkMemoryPropertyFlags optimalMemoryProperties = 0,
	      VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED) :
		Image()
	{
		init(device, width, height, format, tiling, usage, requiredMemoryProperties,
		     optimalMemoryProperties, initialLayout);
	}
	~Image();

	void init(NotNull<Device> device, uint32_t width, uint32_t height, VkFormat format,
		  VkImageTiling tiling, VkImageUsageFlags usage,
		  VkMemoryPropertyFlags requiredMemoryProperties,
		  VkMemoryPropertyFlags optimalMemoryProperties = 0,
		  VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);

	void* map(VkDeviceSize offset, VkDeviceSize size);
	void unmap(bool writeBack = true);
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
	VkMemoryPropertyFlags getMemoryProperties() const { return memoryProperties; }
	VkDeviceSize getMemorySize() const { return memorySize; }
	bool isReady() const { return handle != VK_NULL_HANDLE; }
private:
	friend class Buffer;

	Device* device;
	VkImage handle;
	VkCommandPool cmdPool;
	uint32_t width, height;
	VkFormat format;
	VkImageTiling tiling;
	VkImageUsageFlags usage;
	VkImageLayout layout;
	VkAccessFlags accessFlags;
	VkMemoryPropertyFlags memoryProperties;
	VkDeviceSize memorySize;
	VkDeviceMemory memory;
	VkMappedMemoryRange mapped;
	Buffer* stagingBuffer;

	void createCommandPool();
	void assertReady();
};

}

#endif