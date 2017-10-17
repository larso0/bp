#include <bp/DepthStencilAttachment.h>
#include <stdexcept>

using namespace std;

namespace bp
{

DepthStencilAttachment::DepthStencilAttachment(Device& device, uint32_t width, uint32_t height) :
	Attachment(device, VK_FORMAT_D16_UNORM, width, height)
{
	create();
}

DepthStencilAttachment::~DepthStencilAttachment()
{
	vkDestroyImageView(device, imageView, nullptr);
	delete image;
}

void DepthStencilAttachment::resize(uint32_t width, uint32_t height)
{
	if (width == image->getWidth() && height == image->getHeight()) return;
	vkDestroyImageView(device, imageView, nullptr);
	delete image;
	this->width = width;
	this->height = height;
	create();
}

void DepthStencilAttachment::create()
{
	image = new Image(device, width, height, format, VK_IMAGE_TILING_OPTIMAL,
			  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	image->transition(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			  VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image->getHandle();
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.components = {
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY
	};
	viewInfo.subresourceRange = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};

	VkResult result = vkCreateImageView(device, &viewInfo, nullptr, &imageView);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create depth image view.");
}

}