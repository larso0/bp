#include <bp/OffscreenFramebuffer.h>
#include <bp/RenderPass.h>

namespace bp
{

void OffscreenFramebuffer::init(RenderPass& renderPass, uint32_t width, uint32_t height,
				const AttachmentSlot& colorAttachmentSlot,
				const AttachmentSlot& depthAttachmentSlot)
{
	Device& device = renderPass.getDevice();
	colorAttachment.init(device, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
							       | VK_IMAGE_USAGE_SAMPLED_BIT,
			     width, height);
	depthAttachment.init(device, VK_FORMAT_D16_UNORM,
			     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
			     | VK_IMAGE_USAGE_SAMPLED_BIT, width, height);
	depthAttachment.setClearValue({1.f, 0.f});

	setAttachment(colorAttachmentSlot, colorAttachment);
	setAttachment(depthAttachmentSlot, depthAttachment);

	Framebuffer::init(renderPass, width, height);
}

void OffscreenFramebuffer::resize(uint32_t width, uint32_t height)
{
	colorAttachment.resize(width, height);
	depthAttachment.resize(width, height);
	Framebuffer::resize(width, height);
}

}