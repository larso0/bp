#ifndef BP_IMAGEATTACHMENT_H
#define BP_IMAGEATTACHMENT_H

#include "Attachment.h"
#include "Pointer.h"
#include "Image.h"

namespace bp
{

class ImageAttachment : public Attachment
{
public:
	ImageAttachment() :
		Attachment{},
		imageUsage{0},
		image{nullptr},
		imageView{VK_NULL_HANDLE} {}
	ImageAttachment(NotNull<Device> device, VkFormat format, uint32_t width, uint32_t height) :
		ImageAttachment{}
	{
		init(device, format, width, height);
	}

	virtual ~ImageAttachment();

	void init(NotNull<Device> device, VkFormat format, uint32_t width, uint32_t height);

	void resize(uint32_t width, uint32_t height) override;

	const VkImageView getImageView() const { return imageView; }

protected:
	VkImageUsageFlags imageUsage;
	virtual void createImageView() = 0;
	Image* image;
	VkImageView imageView;

private:
	void create();
	void destroy();
};

}

#endif