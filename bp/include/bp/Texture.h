#ifndef BP_TEXTURE_H
#define BP_TEXTURE_H

#include "Attachment.h"
#include "Image.h"

namespace bp
{

class Texture : public Attachment
{
public:
	Texture() :
		Attachment{},
		imageUsage{0},
		image{nullptr},
		imageView{VK_NULL_HANDLE},
		renderLayout{VK_IMAGE_LAYOUT_UNDEFINED},
		renderAccessFlags{0},
		renderPipelineStage{0} {}
	Texture(Device& device, VkFormat format, VkImageUsageFlags usage,
			uint32_t width, uint32_t height) :
		Texture{}
	{
		init(device, format, usage, width, height);
	}

	virtual ~Texture();

	virtual void init(Device& device, VkFormat format, VkImageUsageFlags usage, uint32_t width,
			  uint32_t height);
	void resize(uint32_t width, uint32_t height) override;
	void transitionShaderReadable(VkCommandBuffer cmdBuffer, VkPipelineStageFlags stage);
	void before(VkCommandBuffer cmdBuffer) override;

	VkImageUsageFlags getImageUsage() const { return imageUsage; }
	Image& getImage() { return *image; }
	VkImageView getImageView() { return imageView; }
	VkImageLayout getInitialLayout() const override { return renderLayout; }
	VkImageLayout getFinalLayout() const override { return renderLayout; }

private:
	VkImageUsageFlags imageUsage;
	Image* image;
	VkImageView imageView;

	VkImageLayout renderLayout;
	VkAccessFlags renderAccessFlags;
	VkPipelineStageFlags renderPipelineStage;

	void create();
	void destroy();
};

}

#endif