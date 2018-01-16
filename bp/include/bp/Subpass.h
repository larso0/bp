#ifndef BP_SUBPASS_H
#define BP_SUBPASS_H

#include "Attachment.h"
#include "Texture.h"
#include <vector>
#include <utility>

namespace bp
{

class RenderPass;

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

	virtual void init(RenderPass& renderPass) = 0;
	virtual void render(VkCommandBuffer cmdBuffer) = 0;

	void addDependency(Subpass& subpass, const DependencyInfo& dependencyInfo);
	void addInputAttachment(Attachment& attachment);
	void addColorAttachment(Attachment& attachment);
	void addColorAttachment(Attachment& attachment, Attachment& resolveAttachment);
	void setDepthAttachment(Texture& depthAttachment);

protected:
	Device* device;

private:
	std::vector<std::pair<Subpass*, DependencyInfo>> dependencies;
	std::vector<Subpass*> dependents;
	std::vector<Attachment*> inputAttachments;
	std::vector<Attachment*> colorAttachments;
	std::vector<Attachment*> resolveAttachments;
	Texture* depthAttachment;

};

}

#endif