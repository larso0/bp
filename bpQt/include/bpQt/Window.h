#ifndef BP_QT_WINDOW_H
#define BP_QT_WINDOW_H

#include "DeviceQuery.h"
#include <bp/Pointer.h>
#include <bp/Swapchain.h>
#include <QWindow>
#include <QVulkanInstance>
#include <vulkan/vulkan.h>

namespace bpQt
{

class Window : public QWindow
{
public:
	Window() :
		surfaceDestroyed{false},
		resizedSinceLastFrame{false},
		surface{VK_NULL_HANDLE},
		frameCmdBuffer{VK_NULL_HANDLE},
		renderCompleteSem{VK_NULL_HANDLE},
		frameCmdBufferBeginInfo{},
		frameSubmitInfo{}
	{
		setSurfaceType(VulkanSurface);
	}
	explicit Window(bp::NotNull<QVulkanInstance> instance) :
		Window{}
	{
		setVulkanInstance(instance);
	}

	virtual ~Window();

	void setSwapchainFlags(const bp::FlagSet<bp::Swapchain::Flags>& flags)
	{
		swapchainFlags = flags;
	}

protected:
	bp::Swapchain swapchain;
	bp::Device device;

	virtual void initRenderResources() = 0;
	virtual void resizeRenderResources(int width, int height) {}
	virtual void specifyDeviceRequirements(bp::DeviceRequirements& requirements) {}
	virtual VkPhysicalDevice selectDevice(const std::vector<VkPhysicalDevice>& devices);
	virtual void render(VkCommandBuffer cmdBuffer) = 0;

private:
	bool surfaceDestroyed, resizedSinceLastFrame;
	VkSurfaceKHR surface;
	bp::FlagSet<bp::Swapchain::Flags> swapchainFlags;

	VkCommandBuffer frameCmdBuffer;
	VkSemaphore renderCompleteSem;

	VkCommandBufferBeginInfo frameCmdBufferBeginInfo;
	VkSubmitInfo frameSubmitInfo;

	void presentQueued()
	{
		vulkanInstance()->presentQueued(this);
	}

	void init();
	void frame();
	void exposeEvent(QExposeEvent*) override;
	void resizeEvent(QResizeEvent* event) override;
	bool event(QEvent* event) override;
};

}

#endif