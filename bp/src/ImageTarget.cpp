#include <bp/ImageTarget.h>
#include <bp/Util.h>
#include <stdexcept>

using namespace std;

namespace bp
{

ImageTarget::ImageTarget(Device& device, uint32_t width, uint32_t height,
			 const FlagSet<Flags>& flags) :
	RenderTarget(device, VK_FORMAT_R8G8B8A8_UNORM, width, height, flags & Flags::DEPTH_IMAGE),
	flags{flags},
	stagingImage{nullptr}
{
	framebufferImageCount = 1;
	createImage();
	if (flags & Flags::STAGING_IMAGE) createStagingImage();

	VkCommandBuffer cmdBuffer = beginSingleUseCmdBuffer(device, cmdPool);
	image->transition(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
			  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, cmdBuffer);
	stagingImage->transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
				 VK_PIPELINE_STAGE_TRANSFER_BIT, cmdBuffer);
	endSingleUseCmdBuffer(device, device.getGraphicsQueue(), cmdPool, cmdBuffer);
}

ImageTarget::~ImageTarget()
{
	vkDestroyImageView(device, framebufferImageViews[0], nullptr);
	delete image;
	if (flags & Flags::STAGING_IMAGE) delete stagingImage;
}

void ImageTarget::beginFrame(VkCommandBuffer cmdBuffer)
{
	image->transition(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
			  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, cmdBuffer);
}

void ImageTarget::endFrame(VkCommandBuffer cmdBuffer)
{
	image->transition(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT,
			  VK_PIPELINE_STAGE_TRANSFER_BIT, cmdBuffer);
}

void ImageTarget::present(VkSemaphore renderCompleteSemaphore)
{
	VkCommandBuffer cmdBuffer = beginSingleUseCmdBuffer(device, cmdPool);
	stagingImage->transfer(*image, cmdBuffer);
	stagingImage->transition(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_HOST_READ_BIT,
				 VK_PIPELINE_STAGE_HOST_BIT, cmdBuffer);

	vkEndCommandBuffer(cmdBuffer);

	VkPipelineStageFlags waitStages = {VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &renderCompleteSemaphore;
	submitInfo.pWaitDstStageMask = &waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &presentSemaphore;

	vkQueueSubmit(device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device.getGraphicsQueue());
}

void ImageTarget::resize(uint32_t width, uint32_t height)
{
	RenderTarget::resize(width, height);
	vkDestroyImageView(device, framebufferImageViews[0], nullptr);
	delete image;
	createImage();
	if (flags & Flags::STAGING_IMAGE)
	{
		delete stagingImage;
		createStagingImage();
	}
}

void ImageTarget::createImage()
{
	image = new Image(device, width, height, format, VK_IMAGE_TILING_OPTIMAL,
			  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
			  (flags & Flags::STAGING_IMAGE ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0) |
			  (flags & Flags::SHADER_READABLE ? VK_IMAGE_USAGE_SAMPLED_BIT : 0),
			  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	framebufferImageViews.resize(1);

	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = image->getHandle();
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = format;
	imageViewInfo.components = {
		VK_COMPONENT_SWIZZLE_R,
		VK_COMPONENT_SWIZZLE_G,
		VK_COMPONENT_SWIZZLE_B,
		VK_COMPONENT_SWIZZLE_A
	};
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;

	VkResult result = vkCreateImageView(device, &imageViewInfo, nullptr,
					    framebufferImageViews.data());
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create image view.");
}

void ImageTarget::createStagingImage()
{
	stagingImage = new Image(device, width, height, format, VK_IMAGE_TILING_LINEAR,
				 VK_IMAGE_USAGE_TRANSFER_DST_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 0,
				 VK_IMAGE_LAYOUT_PREINITIALIZED);

	stagingImage->transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
				 VK_PIPELINE_STAGE_TRANSFER_BIT);
}

}