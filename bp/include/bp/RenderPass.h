#ifndef BP_RENDERPASS_H
#define BP_RENDERPASS_H

#include "RenderTarget.h"

namespace bp
{

class RenderPass
{
public:
	RenderPass() :
		renderTarget(nullptr),
		handle(VK_NULL_HANDLE),
		renderArea({}),
		clearEnabled(false),
		clearValue({}) {}
	~RenderPass();

	void init();
	void recreateFramebuffers();
	void begin(VkCommandBuffer cmdBuffer);
	void end(VkCommandBuffer cmdBuffer);

	void setRenderTarget(RenderTarget* renderTarget);
	void setRenderArea(VkRect2D renderArea);
	void setClearEnabled(bool enabled);
	void setClearValue(VkClearValue clearValue);

	RenderTarget* getRenderTarget() { return renderTarget; }
	VkRenderPass getHandle() { return handle; }
	const VkRect2D& getRenderArea() const { return renderArea; }
	bool isClearEnabled() const { return clearEnabled; }
	const VkClearValue& getClearValue() const { return clearValue; }
	bool isReady() const { return handle != VK_NULL_HANDLE; }
private:
	RenderTarget* renderTarget;
	VkRenderPass handle;
	VkRect2D renderArea;
	bool clearEnabled;
	VkClearValue clearValue;
	std::vector<VkFramebuffer> framebuffers;

	void createFramebuffers();
};

}

#endif