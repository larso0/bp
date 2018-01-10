#ifndef BP_RENDERPASS_H
#define BP_RENDERPASS_H

#include "Subpass.h"
#include "Swapchain.h"
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
		swapchain{nullptr} {}
	~RenderPass();

	void addSubpassGraph(Subpass& subpass);
	void init(uint32_t width, uint32_t height);
	void resize(uint32_t width, uint32_t height);
	void render(VkCommandBuffer cmdBuffer);

	void setRenderArea(const VkRect2D& renderArea)
	{
		RenderPass::renderArea = renderArea;
	}

	operator VkRenderPass() { return handle; }

	VkRenderPass getHandle() { return handle; }
	const VkRect2D& getRenderArea() const { return renderArea; }
	bool isReady() const { return handle != VK_NULL_HANDLE; }
private:
	Device* device;
	VkRenderPass handle;
	VkExtent2D renderExtent;
	VkRect2D renderArea;
	std::vector<VkFramebuffer> framebuffers;

	Swapchain* swapchain;
	std::vector<Attachment*> attachments;
	std::vector<Subpass*> subpasses;
	std::vector<VkAttachmentDescription> attachmentDescriptions;
	std::vector<std::vector<VkAttachmentReference>> attachmentReferences;
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