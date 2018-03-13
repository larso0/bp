#ifndef BP_RENDERER_H
#define BP_RENDERER_H

#include <bp/Framebuffer.h>
#include <bp/RenderPass.h>

namespace bpMulti
{

class Renderer
{
public:
	Renderer() :
		device{nullptr} {}
	virtual ~Renderer() = default;

	void init(bp::Device& device, VkFormat colorFormat, uint32_t width, uint32_t height);

	virtual void init(uint32_t width, uint32_t height) = 0;
	virtual void resize(uint32_t width, uint32_t height);
	virtual void render(bp::Framebuffer& fbo, VkCommandBuffer cmdBuffer);

	bp::Device& getDevice() { return *device; }
	const bp::AttachmentSlot& getColorAttachmentSlot() const { return colorAttachmentSlot; }
	const bp::AttachmentSlot& getDepthAttachmentSlot() const { return depthAttachmentSlot; }
	bp::RenderPass& getRenderPass() { return renderPass; }
	bool isReady() const { return device != nullptr; }

protected:
	void addSubpassGraph(bp::Subpass& subpass);

private:
	bp::Device* device;
	bp::AttachmentSlot colorAttachmentSlot;
	bp::AttachmentSlot depthAttachmentSlot;
	bp::RenderPass renderPass;
};

}

#endif