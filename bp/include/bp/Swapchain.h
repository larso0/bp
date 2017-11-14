#ifndef BP_SWAPCHAIN_H
#define BP_SWAPCHAIN_H

#include "RenderTarget.h"
#include "FlagSet.h"
#include <vector>

namespace bp
{

class Swapchain : public RenderTarget
{
public:
	enum class Flags : size_t
	{
		DEPTH_IMAGE,
		VERTICAL_SYNC,
		BP_FLAGSET_LAST
	};

	Swapchain() :
		RenderTarget{},
		surface{VK_NULL_HANDLE},
		handle{VK_NULL_HANDLE},
		colorSpace{VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} {}
	Swapchain(NotNull<Device> device, VkSurfaceKHR surface, uint32_t width, uint32_t height,
		  const FlagSet<Flags>& flags =
		  	FlagSet<Flags>() << Flags::DEPTH_IMAGE << Flags::VERTICAL_SYNC) :
		Swapchain{}
	{
		init(device, surface, width, height, flags);
	}

	~Swapchain() override;

	void init(NotNull<Device> device, VkSurfaceKHR surface, uint32_t width, uint32_t height,
		  const FlagSet<Flags>& flags =
		  FlagSet<Flags>() << Flags::DEPTH_IMAGE << Flags::VERTICAL_SYNC);

	void beginFrame(VkCommandBuffer cmdBuffer) override;
	void endFrame(VkCommandBuffer cmdBuffer) override;
	void present(VkSemaphore waitSemaphore) override;
	void resize(uint32_t width, uint32_t height) override;

	operator VkSwapchainKHR() { return handle; }

	VkSwapchainKHR getHandle() { return handle; }
	VkSurfaceKHR getSurface() { return surface; }

private:
	FlagSet<Flags> flags;
	VkSurfaceKHR surface;
	VkSwapchainKHR handle;
	VkColorSpaceKHR colorSpace;
	std::vector<VkImage> framebufferImages;
	std::vector<bool> transitionStatus;

	void create();
	void nextImage();
	void transitionColor(VkCommandBuffer cmdBuffer);
	void transitionPresent(VkCommandBuffer cmdBuffer);
};

}

#endif