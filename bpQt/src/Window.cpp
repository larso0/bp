#include <bpQt/Window.h>
#include <stdexcept>
#include <QPlatformSurfaceEvent>
#include <QCoreApplication>

using namespace std;
using namespace bp;

namespace bpQt
{

VkPhysicalDevice Window::selectDevice(const vector<VkPhysicalDevice>& devices)
{
	if (devices.empty()) return VK_NULL_HANDLE;
	return devices[0];
}

void Window::init()
{
	surface = vulkanInstance()->surfaceForWindow(this);

	DeviceRequirements requirements;
	requirements.queues |= VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
	requirements.surface = surface;
	requirements.extensions.push_back("VK_KHR_swapchain");
	specifyDeviceRequirements(requirements);
	
	VkPhysicalDevice physical = selectDevice(queryDevices(*vulkanInstance(), requirements));
	if (physical == VK_NULL_HANDLE) throw runtime_error("No suitable device available.");
	device.init(physical, requirements);
	swapchain.init(device, surface, static_cast<uint32_t>(width()),
		       static_cast<uint32_t>(height()), vsync);
	graphicsQueue = &device.getGraphicsQueue();

	bp::connect(swapchain.presentQueuedEvent, *this, &Window::presentQueued);

	cmdPool.init(device.getGraphicsQueue());
	frameCmdBuffer = cmdPool.allocateCommandBuffer();

	renderCompleteSem.init(device);

	frameCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	frameCmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	initRenderResources();
	inited = true;
}

void Window::frame()
{
	if (!swapchain.isReady()) return;
	if (resized)
	{
		swapchain.resize(static_cast<uint32_t>(width()),
				 static_cast<uint32_t>(height()));
		resizeRenderResources(width(), height());
	}

	vkBeginCommandBuffer(frameCmdBuffer, &frameCmdBufferBeginInfo);
	render(frameCmdBuffer);
	vkEndCommandBuffer(frameCmdBuffer);

	graphicsQueue->submit({{swapchain.getImageAvailableSemaphore(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT}},
		     {frameCmdBuffer}, {renderCompleteSem});
	graphicsQueue->waitIdle();
	swapchain.present(renderCompleteSem);

	if (continuousAnimation)
		requestUpdate();
}

void Window::resizeEvent(QResizeEvent* event)
{
	if (swapchain.isReady()) resized = true;
	QWindow::resizeEvent(event);
}

void Window::exposeEvent(QExposeEvent* event)
{
	if (isExposed())
	{
		if (!inited)
		{
			init();
		} else if (!swapchain.isReady())
		{
			if (surfaceDestroyed)
			{
				surface = vulkanInstance()->surfaceForWindow(this);
				swapchain.recreate(surface);
				surfaceDestroyed = false;
			}
			else swapchain.recreate();
		}
		frame();
	} else
	{
		swapchain.destroy();
	}
	QWindow::exposeEvent(event);
}

bool Window::event(QEvent* event)
{
	switch(event->type())
	{
	case QEvent::UpdateRequest:
		frame();
		break;
	case QEvent::PlatformSurface:
		if (static_cast<QPlatformSurfaceEvent*>(event)->surfaceEventType()
		    == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed)
		{
			swapchain.destroy();
			surfaceDestroyed = true;
		}
		break;
	default:
		break;
	}

	return QWindow::event(event);
}

}