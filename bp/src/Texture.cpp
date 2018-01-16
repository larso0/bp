#include <bp/Texture.h>

using namespace std;

namespace bp
{

Texture::~Texture()
{
	if (isReady()) destroy();
}

void Texture::init(Device& device, VkFormat format, VkImageUsageFlags usage, uint32_t width,
			   uint32_t height)
{
	Attachment::device = &device;
	Attachment::format = format;
	Attachment::width = width;
	Attachment::height = height;
	imageUsage = usage;

	if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		renderLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		renderAccessFlags = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
				    | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		renderPipelineStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	} else if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		renderLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		renderAccessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
				    | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		renderPipelineStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	}

	create();
}

void Texture::resize(uint32_t width, uint32_t height)
{
	if (width != Attachment::width || height != Attachment::height)
	{
		destroy();
		Attachment::width = width;
		Attachment::height = height;
		create();
	}
}

void Texture::transitionShaderReadable(VkCommandBuffer cmdBuffer, VkPipelineStageFlags stage)
{
	image->transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT,
			  stage, cmdBuffer);
}

void Texture::before(VkCommandBuffer cmdBuffer)
{
	image->transition(renderLayout, renderAccessFlags, renderPipelineStage, cmdBuffer);
}

void Texture::create()
{
	image = new Image(*device, width, height, format, VK_IMAGE_TILING_OPTIMAL, imageUsage,
			  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.image = *image;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = format;

	if (format == VK_FORMAT_D16_UNORM)
	{
		imageViewInfo.components = {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};
	} else
	{
		imageViewInfo.components = {
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
	}
	imageViewInfo.subresourceRange.aspectMask = format != VK_FORMAT_D16_UNORM
						    ? VK_IMAGE_ASPECT_COLOR_BIT
						    : VK_IMAGE_ASPECT_DEPTH_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;

	VkResult result = vkCreateImageView(*device, &imageViewInfo, nullptr, &imageView);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create image view.");
}

void Texture::destroy()
{
	vkDestroyImageView(*device, imageView, nullptr);
	delete image;
}

}