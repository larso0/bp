#ifndef BP_IMAGE_H
#define BP_IMAGE_H

#include <vulkan/vulkan.h>

namespace bp
{

class Image
{
public:
	Image() :
		physicalDevice(VK_NULL_HANDLE),
		device(VK_NULL_HANDLE),
		handle(VK_NULL_HANDLE),
		width(0), height(0),
		format(VK_FORMAT_UNDEFINED),
		tiling(VK_IMAGE_TILING_LINEAR),
		layout(VK_IMAGE_LAYOUT_UNDEFINED),
		accessFlags(0),
		usage(0),
		memoryProperties(0),
		memorySize(0),
		memory(VK_NULL_HANDLE),
		mapped({}) {}
	~Image();

	void init();
	void* map(VkDeviceSize offset, VkDeviceSize size);
	void unmap();
	void transition(VkImageLayout dstLayout, VkAccessFlags dstAccess,
			VkPipelineStageFlags dstStage, VkCommandBuffer cmdBuffer);
	void transfer(Image& fromImage, VkCommandBuffer cmdBuffer);

	void setDevice(VkPhysicalDevice physical, VkDevice logical);
	void setSize(uint32_t width, uint32_t height);
	void setFormat(VkFormat format);
	void setTiling(VkImageTiling tiling);
	void setLayout(VkImageLayout layout);
	void setAccessFlags(VkAccessFlags flags);
	void setUsage(VkImageUsageFlags usage);
	void setMemoryProperties(VkMemoryPropertyFlags properties);

	bool isReady() const { return handle != VK_NULL_HANDLE; }
	VkImage getHandle() { return handle; }
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	VkFormat getFormat() const { return format; }
	VkImageTiling getTiling() const { return tiling; }
	VkImageLayout getLayout() const { return layout; }
	VkMemoryPropertyFlags getMemoryProperties() const { return memoryProperties; }
private:
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkImage handle;
	uint32_t width, height;
	VkFormat format;
	VkImageTiling tiling;
	VkImageLayout layout;
	VkAccessFlags accessFlags;
	VkImageUsageFlags usage;
	VkMemoryPropertyFlags memoryProperties;
	VkDeviceSize memorySize;
	VkDeviceMemory memory;
	VkMappedMemoryRange mapped;
};

}

#endif