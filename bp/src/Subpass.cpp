#include <bp/Subpass.h>
#include <stdexcept>

using namespace std;

namespace bp
{

void Subpass::setDepthAttachment(NotNull<DepthAttachment> depthAttachment)
{
	if (!depthAttachment->isReady())
		throw runtime_error("Depth attachment must be initialized before assigning to "
					    "subpass.");
	if (device == nullptr) device = depthAttachment->getDevice();
	else if (device != depthAttachment->getDevice())
		throw runtime_error("Depth attachment must use the same device as the other "
					    "attachments.");
	Subpass::depthAttachment = depthAttachment;
}

void Subpass::addAttachment(NotNull<Attachment> attachment, VkImageLayout layout)
{
	if (!attachment->isReady())
		throw runtime_error("Attachment must be initialized before adding to a subpass.");
	if (device == nullptr) device = attachment->getDevice();
	else if (device != attachment->getDevice())
		throw runtime_error("Attachment must use the same device as the other attachments.");
	attachments.push_back({attachment, layout});
}

}