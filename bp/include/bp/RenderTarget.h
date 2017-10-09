#ifndef BP_RENDERTARGET_H
#define BP_RENDERTARGET_H

#include <vulkan/vulkan.h>
#include <vector>
#include "Image.h"

namespace bp
{

class RenderTarget
{
public:
	RenderTarget() :
		physicalDevice(VK_NULL_HANDLE),
		device(VK_NULL_HANDLE),
		graphicsQueueFamilyIndex(UINT32_MAX),
		graphicsQueue(VK_NULL_HANDLE),
		cmdPool(VK_NULL_HANDLE),
		depthImageEnabled(false),
		depthImage(nullptr),
		depthImageView(VK_NULL_HANDLE),
		format(VK_FORMAT_UNDEFINED),
		width(640), height(480),
		imageCount(1),
		currentImageIndex(0),
		ready(false),
		presentSemaphore(VK_NULL_HANDLE) {}
	virtual ~RenderTarget();

	virtual void init();
	virtual void beginFrame(VkCommandBuffer cmdBuffer) = 0;
	virtual void endFrame(VkCommandBuffer cmdBuffer) = 0;
	virtual void present(VkSemaphore renderCompleteSemaphore) = 0;

	void setDevice(VkPhysicalDevice physical, VkDevice logical);
	void setGraphicsQueue(uint32_t queueFamilyIndex, VkQueue queue);
	void setDepthImageEnabled(bool enabled);
	void setFormat(VkFormat format);
	void setSize(uint32_t width, uint32_t height);
	virtual void resize(uint32_t width, uint32_t height);

	virtual bool isReady() const { return ready; }
	VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
	VkDevice getDevice() { return device; }
	uint32_t getGraphicsQueueFamilyIndex() const { return graphicsQueueFamilyIndex; }
	VkQueue getGraphicsQueue() { return graphicsQueue; }
	VkCommandPool getCmdPool() { return cmdPool; }
	bool isDepthImageEnabled() const { return depthImageEnabled; }
	Image* getDepthImage() { return depthImage; }
	VkImageView getDepthImageView() { return depthImageView; }
	VkFormat getFormat() const { return format; }
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	uint32_t getImageCount() const { return imageCount; }
	std::vector<VkImageView>& getImageViews() { return imageViews; };
	uint32_t getCurrentImageIndex() const { return currentImageIndex; }
	VkSemaphore getPresentSemaphore() { return presentSemaphore; }

protected:
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	uint32_t graphicsQueueFamilyIndex;
	VkQueue graphicsQueue;

	VkCommandPool cmdPool;
	bool depthImageEnabled;
	Image* depthImage;
	VkImageView depthImageView;
	VkFormat format;
	uint32_t width, height;

	uint32_t imageCount;
	std::vector<VkImageView> imageViews;
	uint32_t currentImageIndex;
	VkSemaphore presentSemaphore;

	void createDepthImage();
private:
	bool ready;
};

}

#endif