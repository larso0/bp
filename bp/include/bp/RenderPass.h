#ifndef BP_RENDERPASS_H
#define BP_RENDERPASS_H

#include "Subpass.h"
#include "Swapchain.h"
#include "Pointer.h"
#include <set>
#include <vector>

namespace bp
{

class RenderPass
{
public:
	RenderPass() :
		device{nullptr},
		handle{VK_NULL_HANDLE},
		renderExtent{},
		renderArea{},
		clearEnabled{false},
		swapchain{nullptr} {}
	~RenderPass();

	void addSubpassGraph(NotNull<Subpass> subpass);
	void init();
	void recreateFramebuffers();
	void render(VkCommandBuffer cmdBuffer);

	void setRenderExtent(uint32_t width, uint32_t height)
	{
		renderExtent.width = width;
		renderExtent.height = height;
	}

	void setRenderArea(const VkRect2D& renderArea)
	{
		RenderPass::renderArea = renderArea;
	}

	void setClearEnabled(bool enabled)
	{
		clearEnabled = enabled;
	}

	operator VkRenderPass() { return handle; }

	VkRenderPass getHandle() { return handle; }
	const VkRect2D& getRenderArea() const { return renderArea; }
	bool isClearEnabled() const { return clearEnabled; }
	bool isReady() const { return handle != VK_NULL_HANDLE; }
private:
	Device* device;
	VkRenderPass handle;
	VkExtent2D renderExtent;
	VkRect2D renderArea;
	bool clearEnabled;
	std::vector<VkFramebuffer> framebuffers;

	Swapchain* swapchain;
	std::vector<Attachment*> attachments;
	std::vector<Subpass*> subpasses;
	std::vector<VkAttachmentDescription> attachmentDescriptions;
	std::vector<VkAttachmentReference> attachmentReferences;
	std::vector<VkSubpassDescription> subpassDescriptions;
	std::vector<VkSubpassDependency> subpassDependencies;

	uint32_t getAttachmentIndex(Attachment* a);
	void addAttachment(Attachment* a);
	template <typename Iter>
	void addAttachments(Iter begin, Iter end)
	{
		for (Iter i = begin; i != end; i++) addAttachment(*i);
	}
	const VkAttachmentReference* addAttachmentReference(Attachment* attachment,
							    VkImageLayout layout);
	const VkAttachmentReference* addAttachmentReferences(
		std::vector<Attachment*>& attachments, VkImageLayout layout);

	void create();
	void createFramebuffers();
	void destroyFramebuffers();
};

}

#endif