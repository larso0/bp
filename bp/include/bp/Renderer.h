#ifndef BP_RENDERER_H
#define BP_RENDERER_H

#include "Framebuffer.h"
#include "RenderPass.h"

namespace bp
{

class Renderer
{
public:
	Renderer() :
		device{nullptr} {}
	virtual ~Renderer() = default;

	void init(Device& device, VkFormat colorFormat, uint32_t width, uint32_t height);

	virtual void resize(uint32_t width, uint32_t height);
	virtual void render(Framebuffer& fbo, VkCommandBuffer cmdBuffer);

	Device& getDevice() { return *device; }
	const AttachmentSlot& getColorAttachmentSlot() const { return colorAttachmentSlot; }
	const AttachmentSlot& getDepthAttachmentSlot() const { return depthAttachmentSlot; }
	RenderPass& getRenderPass() { return renderPass; }
	bool isReady() const { return device != nullptr; }

protected:
	virtual void init(uint32_t width, uint32_t height) = 0;
	void addSubpassGraph(Subpass& subpass);

private:
	Device* device;
	AttachmentSlot colorAttachmentSlot;
	AttachmentSlot depthAttachmentSlot;
	RenderPass renderPass;
};

}

#endif