#ifndef BP_ATTACHMENT_H
#define BP_ATTACHMENT_H

#include "Device.h"

namespace bp
{

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
		clearEnabled{false},
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
	bool isClearEnabled() const { return clearEnabled; }
	Device& getDevice() { return *device; }
	VkFormat getFormat() const { return format; }
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	const VkClearValue& getClearValue() const { return clearValue; }
	void setClearEnabled(bool enabled) { clearEnabled = enabled; }
	void setClearValue(const VkClearValue& clearValue) { Attachment::clearValue = clearValue; }

protected:
	Device* device;
	VkFormat format;
	uint32_t width, height;
	bool clearEnabled;
	VkClearValue clearValue;
};

}

#endif