#ifndef BP_SWAPCHAIN_H
#define BP_SWAPCHAIN_H

#include "Attachment.h"
#include "Pointer.h"
#include "Event.h"
#include <vector>

namespace bp
{

class Swapchain : public Attachment
{
public:
	Swapchain() :
		Attachment{},
		vsync{true},
		surface{VK_NULL_HANDLE},
		handle{VK_NULL_HANDLE},
		colorSpace{VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} {}
	Swapchain(NotNull<Device> device, VkSurfaceKHR surface, uint32_t width, uint32_t height,
		  bool vsync = true) :
		Swapchain{}
	{
		init(device, surface, width, height, vsync);
	}

	~Swapchain() override;

	void init(NotNull<Device> device, VkSurfaceKHR surface, uint32_t width, uint32_t height,
		  bool vsync = true);

	void before(VkCommandBuffer cmdBuffer) override;
	void after(VkCommandBuffer cmdBuffer) override;
	void resize(uint32_t width, uint32_t height) override;

	void present(VkSemaphore waitSemaphore);
	void recreate(VkSurfaceKHR newSurface = VK_NULL_HANDLE);
	void destroy();

	operator VkSwapchainKHR() { return handle; }

	bool isReady() const override { handle != VK_NULL_HANDLE; }
	VkSwapchainKHR getHandle() { return handle; }
	VkSurfaceKHR getSurface() { return surface; }
	uint32_t getFramebufferImageCount() const { return framebufferImageCount; }
	uint32_t getCurrentFramebufferIndex() const { return currentFramebufferIndex; }
	VkImage getFramebufferImage(uint32_t i) { return framebufferImages[i]; }
	VkImageView getFramebufferImageView(uint32_t i) { return framebufferImageViews[i]; }
	VkSemaphore getPresentSemaphore() { return presentSemaphore; }

	Event<> presentQueuedEvent;
private:
	bool vsync;
	VkSurfaceKHR surface;
	VkSwapchainKHR handle;
	VkColorSpaceKHR colorSpace;
	uint32_t framebufferImageCount;
	std::vector<VkImage> framebufferImages;
	std::vector<bool> transitionStatus;
	std::vector<VkImageView> framebufferImageViews;
	uint32_t currentFramebufferIndex;
	VkSemaphore presentSemaphore;

	void create();
	void nextImage();
	void transitionColor(VkCommandBuffer cmdBuffer);
	void transitionPresent(VkCommandBuffer cmdBuffer);
};

}

#endif