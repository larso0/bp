#include <bp/RenderTarget.h>
#include <stdexcept>
#include <bp/Util.h>
#include <sstream>

using namespace std;

namespace bp
{

RenderTarget::RenderTarget(Device& device, VkFormat format, uint32_t width, uint32_t height,
			   bool enableDepthImage) :
	device{device},
	format{format},
	width{width}, height{height},
	depthImage{nullptr},
	depthImageView{VK_NULL_HANDLE},
	framebufferImageCount{0},
	currentFramebufferIndex{0}
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolInfo.queueFamilyIndex = device.getGraphicsQueue().getQueueFamilyIndex();

	VkResult result = vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create command pool.");

	VkSemaphoreCreateInfo semInfo =
		{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
	vkCreateSemaphore(device, &semInfo, nullptr, &presentSemaphore);

	if (enableDepthImage) createDepthImage();
}

RenderTarget::~RenderTarget()
{
	vkDestroySemaphore(device, presentSemaphore, nullptr);
	if (isDepthImageEnabled())
	{
		vkDestroyImageView(device, depthImageView, nullptr);
		delete depthImage;
	}
	vkDestroyCommandPool(device, cmdPool, nullptr);
}


void RenderTarget::resize(uint32_t w, uint32_t h)
{
	width = w;
	height = h;
	if (isDepthImageEnabled())
	{
		vkDestroyImageView(device, depthImageView, nullptr);
		delete depthImage;
		createDepthImage();
	}
}

void RenderTarget::createDepthImage()
{
	depthImage = new Image(device, width, height, VK_FORMAT_D16_UNORM, VK_IMAGE_TILING_OPTIMAL,
			       VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	depthImage->transition(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			       VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);

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