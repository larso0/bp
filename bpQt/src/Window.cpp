#include <bpQt/Window.h>
#include <stdexcept>
#include <QPlatformSurfaceEvent>
#include <QCoreApplication>

using namespace std;
using namespace bp;

namespace bpQt
{

static const char* SWAPCHAIN_EXTENSION = "VK_KHR_swapchain";
static const VkPipelineStageFlags WAIT_STAGE = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

Window::~Window()
{
	if (renderCompleteSem != VK_NULL_HANDLE)
		vkDestroySemaphore(device, renderCompleteSem, nullptr);
	if (cmdPool != VK_NULL_HANDLE)
		vkDestroyCommandPool(device, cmdPool, nullptr);
}

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
	requirements.extensions.push_back(SWAPCHAIN_EXTENSION);
	specifyDeviceRequirements(requirements);
	
	VkPhysicalDevice physical = selectDevice(queryDevices(*vulkanInstance(), requirements));
	if (physical == VK_NULL_HANDLE) throw runtime_error("No suitable device available.");
	device.init(physical, requirements);
	swapchain.init(&device, surface, static_cast<uint32_t>(width()),
		       static_cast<uint32_t>(height()), vsync);

	bp::connect(swapchain.presentQueuedEvent, *this, &Window::presentQueued);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = device.getGraphicsQueue()->getQueueFamilyIndex();
	VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &cmdPool);
	if (result != VK_SUCCESS) throw runtime_error("Failed to create command pool.");

	VkCommandBufferAllocateInfo cmdBufferInfo = {};
	cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferInfo.commandPool = cmdPool;
	cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferInfo.commandBufferCount = 1;
	result = vkAllocateCommandBuffers(device, &cmdBufferInfo, &frameCmdBuffer);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to allocate command buffer.");

	VkSemaphoreCreateInfo semInfo = {};
	semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	result = vkCreateSemaphore(device, &semInfo, nullptr, &renderCompleteSem);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create render complete semaphore.");

	frameCmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	frameCmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	frameSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	frameSubmitInfo.commandBufferCount = 1;
	frameSubmitInfo.pCommandBuffers = &frameCmdBuffer;
	frameSubmitInfo.signalSemaphoreCount = 1;
	frameSubmitInfo.pSignalSemaphores = &renderCompleteSem;
	frameSubmitInfo.waitSemaphoreCount = 1;
	frameSubmitInfo.pWaitDstStageMask = &WAIT_STAGE;

	initRenderResources();
	inited = true;
}

void Window::frame()
{
	if (!swapchain.isReady()) return;
	if (swapchain.getWidth() != width() || swapchain.getHeight() != height())
	{
		swapchain.resize(static_cast<uint32_t>(width()),
				 static_cast<uint32_t>(height()));
		resizeRenderResources(width(), height());
	}

	Queue& queue = *device.getGraphicsQueue();
	VkSemaphore presentSem = swapchain.getPresentSemaphore();
	frameSubmitInfo.pWaitSemaphores = &presentSem;

	vkBeginCommandBuffer(frameCmdBuffer, &frameCmdBufferBeginInfo);
	render(frameCmdBuffer);
	vkEndCommandBuffer(frameCmdBuffer);
	vkQueueSubmit(queue, 1, &frameSubmitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);
	swapchain.present(renderCompleteSem);
	if (continuousAnimation)
		QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
}

void Window::exposeEvent(QExposeEvent*)
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
			}
			else swapchain.recreate();
		}
		frame();
	} else
	{
		swapchain.destroy();
	}
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