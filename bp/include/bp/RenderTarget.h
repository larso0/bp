#ifndef BP_RENDERTARGET_H
#define BP_RENDERTARGET_H

#include "Device.h"
#include "Pointer.h"
#include "Image.h"
#include <vector>

namespace bp
{

class RenderTarget
{
public:
	RenderTarget() :
		device{nullptr},
		format{VK_FORMAT_UNDEFINED},
		width{0}, height{0},
		cmdPool{VK_NULL_HANDLE},
		depthImage{VK_NULL_HANDLE},
		depthImageView{VK_NULL_HANDLE},
		framebufferImageCount{0},
		currentFramebufferIndex{0},
		presentSemaphore{VK_NULL_HANDLE} {}
	RenderTarget(NotNull<Device> device, VkFormat format, uint32_t width, uint32_t height,
		     bool enableDepthImage) :
		RenderTarget{}
	{
		init(device, format, width, height, enableDepthImage);
	}
	virtual ~RenderTarget();

	void init(NotNull<Device> device, VkFormat format, uint32_t width, uint32_t height,
		  bool enableDepthImage);

	virtual void beginFrame(VkCommandBuffer cmdBuffer) = 0;
	virtual void endFrame(VkCommandBuffer cmdBuffer) = 0;
	virtual void present(VkSemaphore renderCompleteSemaphore) = 0;
	virtual void resize(uint32_t width, uint32_t height);

	Device& getDevice() { return *device; }
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
	bool isReady() const { return cmdPool != VK_NULL_HANDLE; }

protected:
	Device* device;
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
	void assertReady();
};

}

#endif