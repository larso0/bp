#include <bp/Subpass.h>
#include <stdexcept>

using namespace std;

namespace bp
{

void Subpass::setDepthAttachment(const AttachmentSlot& depthAttachment)
{
	Subpass::depthAttachment = &depthAttachment;
}

void Subpass::addDependency(Subpass& subpass, const DependencyInfo& dependencyInfo)
{
	dependencies.emplace_back(&subpass, dependencyInfo);
	subpass.dependents.push_back(this);
}

void Subpass::addInputAttachment(const AttachmentSlot& attachment, VkImageLayout layout)
{
	inputAttachments.emplace_back(&attachment, layout);
}

void Subpass::addColorAttachment(const AttachmentSlot& attachment)
{
	if (resolveAttachments.size() > 0)
		throw runtime_error("Resolve- and color- attachment counts must match.");
	colorAttachments.push_back(&attachment);
}

void Subpass::addColorAttachment(const AttachmentSlot& attachment,
				 const AttachmentSlot& resolveAttachment)
{
	colorAttachments.push_back(&attachment);

	if (colorAttachments.size() > resolveAttachments.size())
		throw runtime_error("Resolve- and color- attachment counts must match.");
	resolveAttachments.push_back(&resolveAttachment);
}

}