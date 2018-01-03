#ifndef BP_TEXTURE_H
#define BP_TEXTURE_H

#include "ImageAttachment.h"

namespace bp
{

class Texture : public ImageAttachment
{
public:
	Texture() :
		ImageAttachment{}
	{
		imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	Texture(NotNull<Device> device, VkFormat format, uint32_t width, uint32_t height) :
		Texture{}
	{
		init(device, format, width, height);
	}
	virtual ~Texture() = default;

	virtual void before(VkCommandBuffer cmdBuffer) override;


private:
	void createImageView() override;
};

}

#endif