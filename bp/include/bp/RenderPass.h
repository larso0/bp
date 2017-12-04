#ifndef BP_RENDERPASS_H
#define BP_RENDERPASS_H

#include "RenderTarget.h"
#include "Pointer.h"

namespace bp
{

class RenderPass
{
public:
	RenderPass() :
		renderTarget{nullptr},
		handle{VK_NULL_HANDLE},
		renderArea{},
		clearEnabled{false},
		clearValue{} {}
	RenderPass(NotNull<RenderTarget> target, const VkRect2D& area) :
		RenderPass{}
	{
		init(target, area);
	}
	RenderPass(NotNull<RenderTarget> target, const VkRect2D& area,
		   const VkClearValue& clearValue) :
		RenderPass{}
	{
		setClearEnabled(true);
		setClearValue(clearValue);
		init(target, area);
	}
	~RenderPass();

	void init(NotNull<RenderTarget> target, const VkRect2D& area);
	void recreateFramebuffers();
	void begin(VkCommandBuffer cmdBuffer);
	void end(VkCommandBuffer cmdBuffer);

	void setRenderArea(VkRect2D renderArea);
	void setClearEnabled(bool enabled);
	void setClearValue(const VkClearValue& clearValue);

	operator VkRenderPass() { return handle; }

	RenderTarget& getRenderTarget() { return *renderTarget; }
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

	void create();
	void createFramebuffers();
};

}

#endif