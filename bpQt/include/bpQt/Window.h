#ifndef BP_QT_WINDOW_H
#define BP_QT_WINDOW_H

#include "DeviceQuery.h"
#include <bp/Swapchain.h>
#include <bp/CommandPool.h>
#include <bp/Semaphore.h>
#include <QWindow>
#include <QVulkanInstance>
#include <chrono>

namespace bpQt
{

class Window : public QWindow
{
public:
	Window() :
		surfaceDestroyed{false},
		inited{false},
		continuousAnimation{false},
		surface{VK_NULL_HANDLE},
		vsync{true},
		resized{false},
		graphicsQueue{nullptr},
		frameCmdBuffer{VK_NULL_HANDLE},
		frameCmdBufferBeginInfo{}
	{
		setSurfaceType(VulkanSurface);
	}
	explicit Window(QVulkanInstance& instance) :
		Window{}
	{
		setVulkanInstance(&instance);
	}

	virtual ~Window() = default;

	void setVSync(bool enabled) { vsync = enabled; }
	void setContinuousRendering(bool enabled) { continuousAnimation = enabled; }

protected:
	bp::Device device;
	bp::Swapchain swapchain;

	virtual void initRenderResources(uint32_t width, uint32_t height) = 0;
	virtual void resizeRenderResources(uint32_t width, uint32_t height) {}
	virtual void specifyDeviceRequirements(bp::DeviceRequirements& requirements) {}
	virtual VkPhysicalDevice selectDevice(const std::vector<VkPhysicalDevice>& devices);
	virtual void render(VkCommandBuffer cmdBuffer) = 0;
	virtual void update(double frameDeltaTime) {}
	VkCommandBuffer getFrameCommandBuffer() { return frameCmdBuffer; }

private:
	bool surfaceDestroyed, inited, continuousAnimation;
	VkSurfaceKHR surface;
	bool vsync;
	bool resized;

	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
	using Duration = std::chrono::duration<double>;

	TimePoint timer;

	bp::Queue* graphicsQueue;
	bp::CommandPool cmdPool;
	VkCommandBuffer frameCmdBuffer;
	bp::Semaphore renderCompleteSem;

	VkCommandBufferBeginInfo frameCmdBufferBeginInfo;

	void presentQueued()
	{
		vulkanInstance()->presentQueued(this);
	}

	void init();
	void frame();
	void resizeEvent(QResizeEvent*) override;
	void exposeEvent(QExposeEvent*) override;
	bool event(QEvent* event) override;
};

}

#endif
