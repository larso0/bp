#ifndef BP_TEXTURE_H
#define BP_TEXTURE_H

#include "Attachment.h"
#include "Image.h"
#include "ImageDescriptor.h"

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
		sampler{VK_NULL_HANDLE},
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

	void init(Device& device, VkFormat format, VkImageUsageFlags usage, uint32_t width,
			  uint32_t height);
	void load(Device& device, VkImageUsageFlags usage, const std::string& path);
	void resize(uint32_t width, uint32_t height) override;
	void transitionShaderReadable(VkCommandBuffer cmdBuffer, VkPipelineStageFlags stage);
	void before(VkCommandBuffer cmdBuffer) override;

	void setDescriptorBinding(uint32_t binding) { descriptor.setBinding(binding); }

	VkImageUsageFlags getImageUsage() const { return imageUsage; }
	Image& getImage() { return *image; }
	VkImageView getImageView() { return imageView; }
	VkImageLayout getInitialLayout() const override { return renderLayout; }
	VkImageLayout getFinalLayout() const override { return renderLayout; }
	const Descriptor& getDescriptor() { return descriptor; }

private:
	VkImageUsageFlags imageUsage;
	Image* image;
	VkImageView imageView;
	VkSampler sampler;
	ImageDescriptor descriptor;

	VkImageLayout renderLayout;
	VkAccessFlags renderAccessFlags;
	VkPipelineStageFlags renderPipelineStage;

	void create();
	void destroy();
};

}

#endif