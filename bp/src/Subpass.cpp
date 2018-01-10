#include <bp/Subpass.h>
#include <stdexcept>

using namespace std;

namespace bp
{

void Subpass::setDepthAttachment(DepthAttachment& depthAttachment)
{
	if (!depthAttachment.isReady())
		throw runtime_error("Depth attachment must be initialized before assigning to "
					    "subpass.");
	if (device == nullptr) device = &depthAttachment.getDevice();
	else if (device != &depthAttachment.getDevice())
		throw runtime_error("Depth attachment must use the same device as the other "
					    "attachments.");
	Subpass::depthAttachment = &depthAttachment;
}

void Subpass::addDependency(Subpass& subpass, const DependencyInfo& dependencyInfo)
{
	if (device == nullptr) device = subpass.device;
	else if (subpass.device != device)
	{
		if (subpass.device == nullptr) subpass.device = device;
		else throw runtime_error(
				"All subpasses of a render pass must use the same device.");
	}
	dependencies.emplace_back(&subpass, dependencyInfo);
	subpass.dependents.push_back(this);
}

void Subpass::addInputAttachment(Attachment& attachment)
{
	if (!attachment.isReady())
		throw runtime_error("Attachment must be initialized before adding to a subpass.");
	if (device == nullptr) device = &attachment.getDevice();
	else if (device != &attachment.getDevice())
		throw runtime_error(
			"Attachment must use the same device as the other attachments.");
	inputAttachments.push_back(&attachment);
}

void Subpass::addColorAttachment(Attachment& attachment)
{
	if (!attachment.isReady())
		throw runtime_error("Attachment must be initialized before adding to a subpass.");
	if (device == nullptr) device = &attachment.getDevice();
	else if (device != &attachment.getDevice())
		throw runtime_error(
			"Attachment must use the same device as the other attachments.");
	colorAttachments.push_back(&attachment);
}

void Subpass::addColorAttachment(Attachment& attachment, Attachment& resolveAttachment)
{
	if (!attachment.isReady())
		throw runtime_error("Attachment must be initialized before adding to a subpass.");
	if (device == nullptr) device = &attachment.getDevice();
	else if (device != &attachment.getDevice())
		throw runtime_error(
			"Attachment must use the same device as the other attachments.");
	colorAttachments.push_back(&attachment);

	if (colorAttachments.size() > resolveAttachments.size())
		throw runtime_error("Resolve- and color- attachment counts must match.");
	if (!resolveAttachment.isReady())
		throw runtime_error(
			"Attachment must be initialized before adding to a subpass.");
	if (device != &resolveAttachment.getDevice())
		throw runtime_error(
			"Attachment must use the same device as the other attachments.");
	resolveAttachments.push_back(&resolveAttachment);
}

}