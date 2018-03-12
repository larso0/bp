#ifndef BP_FRAMEBUFFER_H
#define BP_FRAMEBUFFER_H

#include "Attachment.h"
#include <unordered_map>
#include <vector>

namespace bp
{

class RenderPass;
class Swapchain;

class Framebuffer
{
public:
	Framebuffer() :
		renderPass{nullptr},
		swapchain{nullptr},
		width{0}, height{0} {}
	Framebuffer(RenderPass& renderPass, uint32_t width, uint32_t height) :
		Framebuffer{} { init(renderPass, width, height); }
	virtual ~Framebuffer();

	void setAttachment(const AttachmentSlot& slot, Attachment& attachment);

	void init(RenderPass& renderPass, uint32_t width, uint32_t height);
	void resize(uint32_t width, uint32_t height);
	void before(VkCommandBuffer cmdBuffer);
	void after(VkCommandBuffer cmdBuffer);

	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	std::vector<VkClearValue> getClearValues();
	VkFramebuffer getHandle() const;
	operator VkFramebuffer() const { return getHandle(); }
	bool isReady() const { return renderPass != nullptr; }
private:
	RenderPass* renderPass;
	Swapchain* swapchain;
	uint32_t width, height;
	std::vector<VkFramebuffer> handles;
	std::unordered_map<const AttachmentSlot*, Attachment*> attachments;

	void create();
	void destroy();
};

}

#endif