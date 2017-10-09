#ifndef BP_SWAPCHAIN_H
#define BP_SWAPCHAIN_H

#include "RenderTarget.h"
#include <vector>

namespace bp
{

class Swapchain : public RenderTarget
{
public:
	Swapchain() :
		RenderTarget(),
		surface(VK_NULL_HANDLE),
		handle(VK_NULL_HANDLE),
		colorSpace(VK_COLOR_SPACE_SRGB_NONLINEAR_KHR),
		enableVSync(true)
	{
		format = VK_FORMAT_B8G8R8_UNORM;
		imageCount = 2;
	}
	~Swapchain() override;

	void init() override;
	void beginFrame(VkCommandBuffer cmdBuffer) override;
	void endFrame(VkCommandBuffer cmdBuffer) override;
	void present(VkSemaphore waitSemaphore) override;
	void resize(uint32_t width, uint32_t height) override;

	bool isReady() const override
	{
		return handle != VK_NULL_HANDLE && RenderTarget::isReady();
	}

	void setSurface(VkSurfaceKHR surface);
	void setVSyncEnabled(bool enabled);

	VkSurfaceKHR getSurface() { return surface; }
	VkSwapchainKHR getHandle() { return handle; }

private:
	VkSurfaceKHR surface;
	VkSwapchainKHR handle;
	VkColorSpaceKHR colorSpace;
	std::vector<VkImage> images;
	std::vector<bool> transitionStatus;
	bool enableVSync;

	void nextImage();
	void transitionColor(VkCommandBuffer cmdBuffer);
	void transitionPresent(VkCommandBuffer cmdBuffer);
};

}

#endif