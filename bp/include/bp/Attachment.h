#ifndef BP_ATTACHMENT_H
#define BP_ATTACHMENT_H

#include <vulkan/vulkan.h>
#include <vector>

namespace bp
{
hd 55
class Attachment
{
public:
	Attachment(uint32_t width, uint32_t height, const VkAttachmentDescription& description,
		   VkClearValue clearValue) :
		width{width}, height{height},
		description{description},
		clearValue{clearValue} {}
	virtual Attachment() = default;

	virtual void resize(uint32_t width, uint32_t height) = 0;
	virtual uint32_t getViewCount() = 0;
	virtual uint32_t getCurrentViewIndex() = 0;
	virtual VkImageView getView(uint32_t index) = 0;

	void setClearValue(const VkClearValue& value) { clearValue = value; }

	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	const VkAttachmentDescription& getDescription() const { return description; }
	const VkClearValue& getClearValue() const { return clearValue; }

protected:
	uint32_t width, height;
	VkAttachmentDescription description;
	VkClearValue clearValue;
};

}

#endif