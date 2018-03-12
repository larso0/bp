#ifndef BP_RENDERPASS_H
#define BP_RENDERPASS_H

#include "Subpass.h"
#include "Framebuffer.h"
#include <vector>

namespace bp
{

class RenderPass
{
public:
	RenderPass() :
		device{nullptr},
		handle{VK_NULL_HANDLE},
		renderArea{} {}
	~RenderPass();

	void addSubpassGraph(Subpass& subpass);
	void init(Device& device);
	void render(Framebuffer& framebuffer, VkCommandBuffer cmdBuffer);

	void setRenderArea(const VkRect2D& renderArea)
	{
		RenderPass::renderArea = renderArea;
	}

	operator VkRenderPass() { return handle; }

	VkRenderPass getHandle() { return handle; }
	Device& getDevice() { return *device; }
	const VkRect2D& getRenderArea() const { return renderArea; }
	uint32_t getAttachmentCount() const { return static_cast<uint32_t>(attachmentSlots.size()); }
	const AttachmentSlot& getAttachmentSlot(unsigned i) const { return *attachmentSlots[i]; }
	bool isReady() const { return handle != VK_NULL_HANDLE; }
private:
	Device* device;
	VkRenderPass handle;
	VkRect2D renderArea;

	std::vector<const AttachmentSlot*> attachmentSlots;
	std::vector<Subpass*> subpasses;
	std::vector<VkAttachmentDescription> attachmentDescriptions;
	std::vector<std::vector<VkAttachmentReference>> attachmentReferences;
	std::vector<VkSubpassDescription> subpassDescriptions;
	std::vector<VkSubpassDependency> subpassDependencies;

	uint32_t getAttachmentIndex(const AttachmentSlot* a);
	void addAttachment(const AttachmentSlot* a);
	const VkAttachmentReference* addAttachmentReference(const AttachmentSlot* attachment,
							    VkImageLayout layout);
	const VkAttachmentReference* addAttachmentReferences(
		std::vector<const AttachmentSlot*>& attachments, VkImageLayout layout);
	const VkAttachmentReference* addAttachmentReferences(
		std::vector<std::pair<const AttachmentSlot*, VkImageLayout>>& attachments);

	void create();
};

}

#endif