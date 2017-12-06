#ifndef BP_QT_WINDOW_H
#define BP_QT_WINDOW_H

#include <bp/Pointer.h>
#include <QWindow>
#include <QVulkanInstance>
#include <vulkan/vulkan.h>

namespace bpQt
{

class Window : public QWindow
{
public:
	Window() :
		surface{VK_NULL_HANDLE}
	{
		setSurfaceType(VulkanSurface);
	}
	explicit Window(bp::NotNull<QVulkanInstance> instance) :
		Window{}
	{
		init(instance);
	}

	void init(bp::NotNull<QVulkanInstance> instance)
	{
		setVulkanInstance(instance);
		surface = instance->surfaceForWindow(this);
	}

	void presentQueued()
	{
		vulkanInstance()->presentQueued(this);
	}

private:
	VkSurfaceKHR surface;
};

}

#endif