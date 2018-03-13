#ifndef BP_COLORDEPTHFRAMEBUFFER_H
#define BP_COLORDEPTHFRAMEBUFFER_H

#include "Framebuffer.h"
#include "Texture.h"

namespace bp
{

class OffscreenFramebuffer : public Framebuffer
{
public:
	OffscreenFramebuffer() : Framebuffer{} {}

	void init(RenderPass& renderPass, uint32_t width, uint32_t height,
		  const AttachmentSlot& colorAttachmentSlot,
		  const AttachmentSlot& depthAttachmentSlot);
	void resize(uint32_t width, uint32_t height) override;

	Texture& getColorAttachment() { return colorAttachment; }
	Texture& getDepthAttachment() { return depthAttachment; }

private:
	Texture colorAttachment;
	Texture depthAttachment;
};

}

#endif