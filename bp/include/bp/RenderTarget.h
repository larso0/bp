#ifndef BP_RENDERTARGET_H
#define BP_RENDERTARGET_H

#include <vulkan/vulkan.h>
#include <vector>
#include "Device.h"
#include "Image.h"

namespace bp
{

class RenderTarget
{
public:
	RenderTarget(Device& device, VkFormat format, uint32_t width, uint32_t height,
		     bool enableDepthImage);
	virtual ~RenderTarget();

	virtual void beginFrame(VkCommandBuffer cmdBuffer) = 0;
	virtual void endFrame(VkCommandBuffer cmdBuffer) = 0;
	virtual void present(VkSemaphore renderCompleteSemaphore) = 0;
	virtual void resize(uint32_t width, uint32_t height);

	Device& getDevice() { return device; }
	VkCommandPool getCmdPool() { return cmdPool; }
	bool isDepthImageEnabled() const { return depthImage != nullptr; }
	Image* getDepthImage() { return depthImage; }
	VkImageView getDepthImageView() { return depthImageView; }
	VkFormat getFormat() const { return format; }
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	uint32_t getFramebufferImageCount() const { return framebufferImageCount; }
	std::vector<VkImageView>& getFramebufferImageViews() { return framebufferImageViews; };
	uint32_t getCurrentFramebufferIndex() const { return currentFramebufferIndex; }
	VkSemaphore getPresentSemaphore() { return presentSemaphore; }

protected:
	Device& device;
	VkFormat format;
	uint32_t width, height;

	VkCommandPool cmdPool;
	Image* depthImage;
	VkImageView depthImageView;

	uint32_t framebufferImageCount;
	std::vector<VkImageView> framebufferImageViews;
	uint32_t currentFramebufferIndex;
	VkSemaphore presentSemaphore;

	void createDepthImage();
};

}

#endif