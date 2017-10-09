#include <bp/ImageTarget.h>
#include <bp/Util.h>
#include <stdexcept>

using namespace std;

namespace bp
{

ImageTarget::~ImageTarget()
{
	if (isReady())
	{
		vkDestroyImageView(device, imageViews[0], nullptr);
		delete image;
		delete stagingImage;
	}
}

void ImageTarget::init()
{
	RenderTarget::init();

	createImage();
	createStagingImage();

	VkCommandBuffer cmdBuffer = beginSingleUseCmdBuffer(device, cmdPool);
	image->transition(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
			  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, cmdBuffer);
	stagingImage->transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
				 VK_PIPELINE_STAGE_TRANSFER_BIT, cmdBuffer);
	endSingleUseCmdBuffer(device, graphicsQueue, cmdPool, cmdBuffer);
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

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);
}

void ImageTarget::resize(uint32_t width, uint32_t height)
{
	RenderTarget::resize(width, height);
	if (isReady())
	{
		vkDestroyImageView(device, imageViews[0], nullptr);
		delete image;
		delete stagingImage;
		createImage();
		createStagingImage();
	}
}

void ImageTarget::createImage()
{
	image = new Image();
	image->setDevice(physicalDevice, device);
	image->setSize(width, height);
	image->setFormat(format);
	image->setTiling(VK_IMAGE_TILING_OPTIMAL);
	image->setUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	image->setMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	image->init();

	imageViews.resize(1);

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

	VkResult result = vkCreateImageView(device, &imageViewInfo, nullptr, imageViews.data());
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create image view.");
}

void ImageTarget::createStagingImage()
{
	stagingImage = new Image();
	stagingImage->setDevice(physicalDevice, device);
	stagingImage->setSize(width, height);
	stagingImage->setFormat(format);
	stagingImage->setTiling(VK_IMAGE_TILING_LINEAR);
	stagingImage->setLayout(VK_IMAGE_LAYOUT_PREINITIALIZED);
	stagingImage->setUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	stagingImage->setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	stagingImage->init();

	VkCommandBuffer cmdBuffer = beginSingleUseCmdBuffer(device, cmdPool);
	stagingImage->transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
				 VK_PIPELINE_STAGE_TRANSFER_BIT, cmdBuffer);
	endSingleUseCmdBuffer(device, graphicsQueue, cmdPool, cmdBuffer);
}

}