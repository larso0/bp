#ifndef BP_SUBPASS_H
#define BP_SUBPASS_H

#include "Attachment.h"
#include "DepthAttachment.h"
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
		device{nullptr},
		depthAttachment{nullptr} {}
	virtual ~Subpass() = default;

	virtual void render(VkCommandBuffer cmdBuffer) = 0;

	void addDependency(NotNull<Subpass> subpass, const DependencyInfo& dependencyInfo);
	void addInputAttachment(NotNull<Attachment> attachment);
	void addColorAttachment(NotNull<Attachment> attachment,
				Attachment* resolveAttachment = nullptr);
	void setDepthAttachment(NotNull<DepthAttachment> depthAttachment);

protected:
	Device* device;

private:
	std::vector<std::pair<Subpass*, DependencyInfo>> dependencies;
	std::vector<Subpass*> dependents;
	std::vector<Attachment*> inputAttachments;
	std::vector<Attachment*> colorAttachments;
	std::vector<Attachment*> resolveAttachments;
	DepthAttachment* depthAttachment;

};

}

#endif