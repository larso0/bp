#ifndef BP_DEPTHSTENCILATTACHMENT_H
#define BP_DEPTHSTENCILATTACHMENT_H

#include "Attachment.h"
#include "Device.h"
#include "Image.h"


namespace bp
{

class DepthStencilAttachment : public Attachment
{
public:
	DepthStencilAttachment(Device& device, uint32_t width, uint32_t height);
	~DepthStencilAttachment() override;

	void resize(uint32_t width, uint32_t height) override;

	VkImageView getImageView() { return imageView; }

private:
	Image* image;
	VkImageView imageView;

	void create();
};

}

#endif