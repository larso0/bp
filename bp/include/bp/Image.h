#ifndef BP_IMAGE_H
#define BP_IMAGE_H

#include "Device.h"
#include <vulkan/vulkan.h>

namespace bp
{

class Image
{
public:
	Image(Device& device, uint32_t width, uint32_t height, VkFormat format,
	      VkImageTiling tiling, VkImageUsageFlags usage,
	      VkMemoryPropertyFlags requiredMemoryProperties,
	      VkMemoryPropertyFlags optimalMemoryProperties = 0,
	      VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED);
	~Image();

	void* map(VkDeviceSize offset, VkDeviceSize size);
	void unmap(bool writeBack = true);
	void transition(VkImageLayout dstLayout, VkAccessFlags dstAccess,
			VkPipelineStageFlags dstStage, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);
	void transfer(Image& fromImage, VkCommandBuffer cmdBuffer = VK_NULL_HANDLE);

	VkImage getHandle() { return handle; }
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	VkFormat getFormat() const { return format; }
	VkImageTiling getTiling() const { return tiling; }
	VkImageLayout getLayout() const { return layout; }
	VkMemoryPropertyFlags getMemoryProperties() const { return memoryProperties; }
	VkDeviceSize getMemorySize() const { return memorySize; }
private:
	Device& device;
	VkImage handle;
	VkCommandPool cmdPool;
	uint32_t width, height;
	VkFormat format;
	VkImageTiling tiling;
	VkImageLayout layout;
	VkAccessFlags accessFlags;
	VkMemoryPropertyFlags memoryProperties;
	VkDeviceSize memorySize;
	VkDeviceMemory memory;
	VkMappedMemoryRange mapped;
	Image* stagingImage;

	void createCommandPool();
};

}

#endif