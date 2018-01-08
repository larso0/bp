#ifndef BP_TEXTURE_H
#define BP_TEXTURE_H

#include "ImageAttachment.h"
#include "FlagSet.h"

namespace bp
{

class Texture : public ImageAttachment
{
public:
	enum class UsageFlags
	{
		SHADER_READABLE,
		COLOR_ATTACHMENT,
		BP_FLAGSET_LAST
	};

	Texture() :
		ImageAttachment{}
	{
		usageFlags << UsageFlags::COLOR_ATTACHMENT << UsageFlags::SHADER_READABLE;
		imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	}
	Texture(NotNull<Device> device, VkFormat format, uint32_t width, uint32_t height,
		const FlagSet<UsageFlags>& usageFlags) :
		Texture{}
	{
		setUsageFlags(usageFlags);
		init(device, format, width, height);
	}
	virtual ~Texture() = default;

	void setUsageFlags(const FlagSet<UsageFlags>& usageFlags);

	void before(VkCommandBuffer cmdBuffer) override;
	void transitionShaderReadable(VkCommandBuffer cmdBuffer);

private:
	FlagSet<UsageFlags> usageFlags;

	void createImageView() override;
};

}

#endif