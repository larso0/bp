#ifndef BP_ATTACHMENT_H
#define BP_ATTACHMENT_H

#include "Device.h"

namespace bp
{

class AttachmentSlot
{
public:
	AttachmentSlot() : description{} {}
	AttachmentSlot(VkFormat format, VkSampleCountFlagBits samples,
		       VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
		       VkImageLayout initialLayout, VkImageLayout finalLayout) :
		AttachmentSlot{}
	{
		init(format, samples, loadOp, storeOp, initialLayout, finalLayout);
	}

	void init(VkFormat format, VkSampleCountFlagBits samples,
		  VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
		  VkImageLayout initialLayout, VkImageLayout finalLayout)
	{
		description.format = format;
		description.samples = samples;
		description.loadOp = loadOp;
		description.storeOp = storeOp;
		description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		description.initialLayout = initialLayout;
		description.finalLayout = finalLayout;
	}

	const VkAttachmentDescription& getDescription() const { return description; }
private:
	VkAttachmentDescription description;
};

/*
 * Attachment to a render pass
 */
class Attachment
{
public:
	Attachment() :
		device{nullptr},
		format{VK_FORMAT_UNDEFINED},
		width{0}, height{0},
		clearValue{} {}
	virtual ~Attachment() = default;

	/*
	 * Override to record commands before the render pass.
	 */
	virtual void before(VkCommandBuffer cmdBuffer) {}

	/*
	 * Override to record commands after the render pass.
	 */
	virtual void after(VkCommandBuffer cmdBuffer) {}

	virtual void resize(uint32_t width, uint32_t height) = 0;

	virtual bool isReady() const { return device != nullptr; }
	Device& getDevice() { return *device; }
	VkFormat getFormat() const { return format; }
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	const VkClearValue& getClearValue() const { return clearValue; }
	void setClearValue(const VkClearValue& clearValue) { Attachment::clearValue = clearValue; }
	virtual VkImageLayout getInitialLayout() const = 0;
	virtual VkImageLayout getFinalLayout() const = 0;

protected:
	Device* device;
	VkFormat format;
	uint32_t width, height;
	VkClearValue clearValue;
};

}

#endif