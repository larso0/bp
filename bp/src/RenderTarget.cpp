#include <bp/RenderTarget.h>
#include <stdexcept>
#include <bp/Util.h>
#include <sstream>

using namespace std;

namespace bp
{

void RenderTarget::init(NotNull<Device> device, VkFormat format, uint32_t width, uint32_t height,
			FlagSet<Flags> flags)
{
	if (isReady()) throw runtime_error("Render target already initialized.");
	if (flags & Flags::DEPTH_STAGING_IMAGE) flags << Flags::DEPTH_IMAGE;
	this->flags = flags;
	this->device = device;
	this->format = format;
	this->width = width;
	this->height = height;

	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	cmdPoolInfo.queueFamilyIndex = device->getGraphicsQueue().getQueueFamilyIndex();

	VkResult result = vkCreateCommandPool(*device, &cmdPoolInfo, nullptr, &cmdPool);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create command pool.");

	VkSemaphoreCreateInfo semInfo =
		{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
	result = vkCreateSemaphore(*device, &semInfo, nullptr, &presentSemaphore);
	if (result != VK_SUCCESS)
	{
		vkDestroyCommandPool(*device, cmdPool, nullptr);
		cmdPool = VK_NULL_HANDLE;
		throw runtime_error("Failed to create present semaphore.");
	}

	if (flags & Flags::DEPTH_IMAGE)
	{
		try
		{
			createDepthImage();
		} catch (exception& e)
		{
			vkDestroySemaphore(*device, presentSemaphore, nullptr);
			vkDestroyCommandPool(*device, cmdPool, nullptr);
			presentSemaphore = VK_NULL_HANDLE;
			cmdPool = VK_NULL_HANDLE;
			throw e;
		}
	}
	if (flags & Flags::DEPTH_STAGING_IMAGE)
	{
		try
		{
			createDepthStagingImage();
		} catch (exception& e)
		{
			vkDestroyImageView(*device, depthImageView, nullptr);
			delete depthImage;
			vkDestroySemaphore(*device, presentSemaphore, nullptr);
			vkDestroyCommandPool(*device, cmdPool, nullptr);
			presentSemaphore = VK_NULL_HANDLE;
			cmdPool = VK_NULL_HANDLE;
			throw e;
		}
	}
}

RenderTarget::~RenderTarget()
{
	if (!isReady()) return;
	vkDestroySemaphore(*device, presentSemaphore, nullptr);
	if (isDepthImageEnabled())
	{
		vkDestroyImageView(*device, depthImageView, nullptr);
		delete depthImage;
	}
	if (flags & Flags::DEPTH_STAGING_IMAGE) delete depthStagingImage;
	vkDestroyCommandPool(*device, cmdPool, nullptr);
}


void RenderTarget::resize(uint32_t w, uint32_t h)
{
	assertReady();
	width = w;
	height = h;
	if (isDepthImageEnabled())
	{
		vkDestroyImageView(*device, depthImageView, nullptr);
		delete depthImage;
		createDepthImage();
	}
	if (flags & Flags::DEPTH_STAGING_IMAGE)
	{
		delete depthStagingImage;
		createDepthStagingImage();
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

	VkResult result = vkCreateImageView(*device, &viewInfo, nullptr, &depthImageView);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create depth image view.");
}

void RenderTarget::assertReady()
{
	if (!isReady())
		throw runtime_error("Renter target not ready. Must be initialized before use.");
}

void RenderTarget::createDepthStagingImage()
{
	depthStagingImage = new Image(device, width, height, VK_FORMAT_D16_UNORM,
				      VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_DST_BIT,
				      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0,
				      VK_IMAGE_LAYOUT_PREINITIALIZED);

	depthStagingImage->transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				      VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
}

}