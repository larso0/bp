#ifndef BP_DEPTHATTACHMENT_H
#define BP_DEPTHATTACHMENT_H

#include "ImageAttachment.h"

namespace bp
{

class DepthAttachment : public ImageAttachment
{
public:
	DepthAttachment() :
		ImageAttachment{}
	{
		imageUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	DepthAttachment(Device& device, uint32_t width, uint32_t height) :
		DepthAttachment{}
	{
		init(device, width, height);
	}
	virtual ~DepthAttachment() = default;

	void init(Device& device, uint32_t width, uint32_t height)
	{
		ImageAttachment::init(device, VK_FORMAT_D16_UNORM, width, height);
	}

	virtual void before(VkCommandBuffer cmdBuffer) override;

private:
	void createImageView() override;
};

}

#endif