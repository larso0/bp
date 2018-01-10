#include <bp/ImageAttachment.h>

using namespace std;

namespace bp
{

ImageAttachment::~ImageAttachment()
{
	if (isReady()) destroy();
}

void ImageAttachment::init(Device& device, VkFormat format, uint32_t width,
			   uint32_t height)
{
	Attachment::device = &device;
	Attachment::format = format;
	Attachment::width = width;
	Attachment::height = height;
	create();
}

void ImageAttachment::resize(uint32_t width, uint32_t height)
{
	if (width != Attachment::width || height != Attachment::height)
	{
		destroy();
		Attachment::width = width;
		Attachment::height = height;
		create();
	}
}

void ImageAttachment::create()
{
	image = new Image(*device, width, height, format, VK_IMAGE_TILING_OPTIMAL, imageUsage,
			  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	createImageView();
}

void ImageAttachment::destroy()
{
	vkDestroyImageView(*device, imageView, nullptr);
	delete image;
}

}