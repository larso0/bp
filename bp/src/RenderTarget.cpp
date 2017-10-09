#include <bp/RenderTarget.h>
#include <stdexcept>
#include <bp/Util.h>
#include <sstream>

using namespace std;

namespace bp
{

RenderTarget::~RenderTarget()
{
	if (ready)
	{
		vkDestroySemaphore(device, presentSemaphore, nullptr);
		if (depthImageEnabled)
		{
			vkDestroyImageView(device, depthImageView, nullptr);
			delete depthImage;
		}
		vkDestroyCommandPool(device, cmdPool, nullptr);
	}
}

void RenderTarget::init()
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;

	VkResult result = vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create command pool.");

	VkSemaphoreCreateInfo semInfo =
		{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
	vkCreateSemaphore(device, &semInfo, nullptr, &presentSemaphore);

	if (depthImageEnabled) createDepthImage();
	ready = true;
}

void RenderTarget::setDevice(VkPhysicalDevice physical, VkDevice logical)
{
	if (isReady())
		throw runtime_error("Failed to alter device, render target already created.");
	physicalDevice = physical;
	device = logical;
}

void RenderTarget::setGraphicsQueue(uint32_t queueFamilyIndex, VkQueue queue)
{
	if (isReady())
		throw runtime_error("Failed to alter queue, render target already created.");
	graphicsQueueFamilyIndex = queueFamilyIndex;
	graphicsQueue = queue;
}

void RenderTarget::setDepthImageEnabled(bool enabled)
{
	if (isReady())
	{
		stringstream ss;
		ss << "Failed to " << (enabled ? "enable" : "disable")
		   << " depth image, render target already created.";
		throw runtime_error(ss.str());
	}
	depthImageEnabled = enabled;
}

void RenderTarget::setFormat(VkFormat format)
{
	if (isReady())
		throw runtime_error("Failed to alter format, render target already created.");
	this->format = format;
}

void RenderTarget::setSize(uint32_t w, uint32_t h)
{
	if (isReady())
		throw runtime_error("Failed to alter size, render target already created.");
	width = w;
	height = h;
}

void RenderTarget::resize(uint32_t w, uint32_t h)
{
	width = w;
	height = h;
	if (ready && depthImageEnabled)
	{
		vkDestroyImageView(device, depthImageView, nullptr);
		delete depthImage;
		createDepthImage();
	}
}

void RenderTarget::createDepthImage()
{
	depthImage = new Image();
	depthImage->setDevice(physicalDevice, device);
	depthImage->setSize(width, height);
	depthImage->setFormat(VK_FORMAT_D16_UNORM);
	depthImage->setTiling(VK_IMAGE_TILING_OPTIMAL);
	depthImage->setUsage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	depthImage->setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	depthImage->init();

	VkCommandBuffer cmdBuffer = beginSingleUseCmdBuffer(device, cmdPool);
	depthImage->transition(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			       VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, cmdBuffer);
	endSingleUseCmdBuffer(device, graphicsQueue, cmdPool, cmdBuffer);

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = depthImage->getHandle();
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_D16_UNORM;
	viewInfo.components = {
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY
	};
	viewInfo.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};

	VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &depthImageView);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create depth image view.");
}

}