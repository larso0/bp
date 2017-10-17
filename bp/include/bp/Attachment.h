#ifndef BP_ATTACHMENT_H
#define BP_ATTACHMENT_H

#include "Device.h"

namespace bp
{

class Attachment
{
public:
	Attachment(Device& device, VkFormat format, uint32_t width, uint32_t height) :
		device{device}
		format{format},
		width{width}, height{height},
		clearValue{} {}
	virtual ~Attachment() = default;

	virtual void beforeRender(VkCommandBuffer cmdBuffer) {}
	virtual void afterRender(VkCommandBuffer cmdBuffer) {}
	virtual void resize(uint32_t width, uint32_t height) = 0;

	void setClearValue(const VkClearValue& value) { clearValue = value; }

	VkFormat getFormat() const { return format; }
	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	const VkClearValue& getClearValue() const { return clearValue; }

protected:
	Device& device;
	VkFormat format;
	uint32_t width, height;
	VkClearValue clearValue;
};

}

#endif