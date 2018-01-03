#include <bp/DepthAttachment.h>
#include <stdexcept>

using namespace std;

namespace bp
{

void DepthAttachment::before(VkCommandBuffer cmdBuffer)
{
	image->transition(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			  VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, cmdBuffer);
}

void DepthAttachment::createImageView()
{
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

	VkResult result = vkCreateImageView(*device, &viewInfo, nullptr, &imageView);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create depth image view.");
}

}