#include <bp/Framebuffer.h>
#include <bp/RenderPass.h>
#include <bp/Swapchain.h>
#include <bp/Texture.h>
#include <stdexcept>

using namespace std;

namespace bp
{

Framebuffer::~Framebuffer()
{
	if (isReady()) destroy();
}

void Framebuffer::setAttachment(const AttachmentSlot& slot, Attachment& attachment)
{
	Swapchain* swapchain = dynamic_cast<Swapchain*>(&attachment);
	if (swapchain != nullptr)
	{
		if (Framebuffer::swapchain != nullptr)
			throw invalid_argument{"Only one swapchain attachment can be used "
					       "per framebuffer."};
		Framebuffer::swapchain = swapchain;
	}

	attachments[&slot] = &attachment;
}

void Framebuffer::init(RenderPass& renderPass, uint32_t width, uint32_t height)
{
	if (isReady())
		throw runtime_error("Framebuffer already initialized.");

	Framebuffer::renderPass = &renderPass;
	Framebuffer::width = width;
	Framebuffer::height = height;

	try { create(); } catch (exception& e)
	{
		Framebuffer::renderPass = nullptr;
		throw e;
	}
}

void Framebuffer::resize(uint32_t width, uint32_t height)
{
	Framebuffer::width = width;
	Framebuffer::height = height;

	destroy();
	create();
}

void Framebuffer::before(VkCommandBuffer cmdBuffer)
{
	for (auto& a : attachments)
		a.second->before(cmdBuffer);
}

void Framebuffer::after(VkCommandBuffer cmdBuffer)
{
	for (auto& a : attachments)
		a.second->after(cmdBuffer);
}

std::vector<VkClearValue> Framebuffer::getClearValues()
{
	std::vector<VkClearValue> clearValues;
	for (uint32_t i = 0; i < renderPass->getAttachmentCount(); i++)
	{
		clearValues.push_back(
			attachments[&renderPass->getAttachmentSlot(i)]->getClearValue());
	}
	return clearValues;
}

VkFramebuffer Framebuffer::getHandle() const
{
	if (swapchain != nullptr) return handles[swapchain->getCurrentFramebufferIndex()];
	return handles[0];
}

void Framebuffer::create()
{
	vector<VkImageView> attachmentViews;
	int swapchainIndex = -1;
	for (uint32_t i = 0; i < renderPass->getAttachmentCount(); i++)
	{
		Attachment* attachment = nullptr;

		try
		{
			attachment = attachments[&renderPass->getAttachmentSlot(i)];
		} catch (...)
		{
			throw runtime_error("Missing attachments during framebuffer creation.");
		}

		if (swapchain != nullptr && attachment == static_cast<Attachment*>(swapchain))
		{
			swapchainIndex = i;
			attachmentViews.push_back(VK_NULL_HANDLE);
			continue;
		}
		auto texture = dynamic_cast<Texture*>(attachment);
		if (texture == nullptr)
			throw runtime_error("Unsupported attachment implementation.");
		attachmentViews.push_back(texture->getImageView());
	}

	VkFramebufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.renderPass = *renderPass;
	info.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
	info.pAttachments = attachmentViews.data();
	info.width = width;
	info.height = height;
	info.layers = 1;

	auto framebufferCount = swapchain != nullptr ? swapchain->getFramebufferImageCount() : 1;
	handles.resize(framebufferCount);
	for (auto i = 0; i < framebufferCount; i++)
	{
		if (swapchain != nullptr)
		{
			attachmentViews[swapchainIndex] =
				swapchain->getFramebufferImageView(static_cast<uint32_t>(i));
		}
		VkResult result = vkCreateFramebuffer(renderPass->getDevice(), &info, nullptr,
						      handles.data() + i);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create framebuffer.");
	}
}

void Framebuffer::destroy()
{
	for (auto fb : handles) vkDestroyFramebuffer(renderPass->getDevice(), fb, nullptr);
}

}