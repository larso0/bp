#include <bp/Texture.h>
#include <stdexcept>

using namespace std;

namespace bp
{

void Texture::setUsageFlags(const FlagSet<UsageFlags>& usageFlags)
{
	if (isReady())
		throw runtime_error("Texture usage flags must be set before initialization.");
	Texture::usageFlags = usageFlags;

	imageUsage = 0;
	if (usageFlags & UsageFlags::SHADER_READABLE)
		imageUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	if (usageFlags & UsageFlags::COLOR_ATTACHMENT)
		imageUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
}

void Texture::before(VkCommandBuffer cmdBuffer)
{
	image->transition(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			  VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
			  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, cmdBuffer);
}

void Texture::transitionShaderReadable(VkCommandBuffer cmdBuffer)
{
	image->transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT,
			  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, cmdBuffer);
}

void Texture::createImageView()
{
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

}