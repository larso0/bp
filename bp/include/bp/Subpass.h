#ifndef BP_SUBPASS_H
#define BP_SUBPASS_H

#include "Attachment.h"
#include "DepthAttachment.h"
#include <vector>

namespace bp
{

class Subpass
{
	friend class RenderPass;
public:
	struct AttachmentReference
	{
		Attachment* attachment;
		VkImageLayout layout;
	};

	Subpass() :
		device{nullptr},
		depthAttachment{nullptr} {}
	virtual ~Subpass() = default;

	virtual void render(VkCommandBuffer cmdBuffer) = 0;

	void setDepthAttachment(NotNull<DepthAttachment> depthAttachment);
	void addAttachment(NotNull<Attachment> attachment, VkImageLayout layout);
	void addSubpass(NotNull<Subpass> subpass)
	{
		dependentSubpasses.push_back(subpass.get());
	}

protected:
	Device* device;

private:
	DepthAttachment* depthAttachment;
	std::vector<AttachmentReference> attachments;
	std::vector<Subpass*> dependentSubpasses;
};

}

#endif