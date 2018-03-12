#ifndef BP_SUBPASS_H
#define BP_SUBPASS_H

#include <bp/Attachment.h>
#include <vector>
#include <utility>

namespace bp
{

class Subpass
{
	friend class RenderPass;
public:
	struct DependencyInfo
	{
		VkPipelineStageFlags srcStageMask;
		VkPipelineStageFlags dstStageMask;
		VkAccessFlags srcAccessMask;
		VkAccessFlags dstAccessMask;
		VkDependencyFlags dependencyFlags;
	};

	Subpass() :
		depthAttachment{nullptr} {}
	virtual ~Subpass() = default;

	virtual void render(const VkRect2D& area, VkCommandBuffer cmdBuffer) = 0;

	void addDependency(Subpass& subpass, const DependencyInfo& dependencyInfo);
	void addInputAttachment(const AttachmentSlot& attachment, VkImageLayout layout);
	void addColorAttachment(const AttachmentSlot& attachment);
	void addColorAttachment(const AttachmentSlot& attachment,
				const AttachmentSlot& resolveAttachment);
	void setDepthAttachment(const AttachmentSlot& depthAttachment);

private:
	std::vector<std::pair<Subpass*, DependencyInfo>> dependencies;
	std::vector<Subpass*> dependents;
	std::vector<std::pair<const AttachmentSlot*, VkImageLayout>> inputAttachments;
	std::vector<const AttachmentSlot*> colorAttachments;
	std::vector<const AttachmentSlot*> resolveAttachments;
	const AttachmentSlot* depthAttachment;

};

}

#endif