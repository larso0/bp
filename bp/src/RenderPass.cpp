#include <bp/RenderPass.h>
#include <bp/Swapchain.h>
#include <stdexcept>
#include <algorithm>

using namespace std;

namespace bp
{

RenderPass::~RenderPass()
{
	if (isReady())
	{
		vkDestroyRenderPass(*device, handle, nullptr);
	}
}

void RenderPass::addSubpassGraph(Subpass& subpass)
{
	subpasses.push_back(&subpass);

	for (auto& a : subpass.inputAttachments) addAttachment(a.first);
	for (auto a : subpass.colorAttachments) addAttachment(a);
	for (auto a : subpass.resolveAttachments) addAttachment(a);
	if (subpass.depthAttachment != nullptr) addAttachment(subpass.depthAttachment);

	for (auto s : subpass.dependents)
	{
		addSubpassGraph(*s);
	}
}

void RenderPass::init(Device& device)
{
	if (isReady())
		throw runtime_error("Render pass already initialized.");

	RenderPass::device = &device;

	if (subpasses.empty())
		throw runtime_error("No subpasses added to render pass.");

	for (const AttachmentSlot* attachment : attachmentSlots)
	{
		attachmentDescriptions.push_back(attachment->getDescription());
	}

	for (Subpass* subpass : subpasses)
	{
		VkSubpassDescription description = {};
		description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		if (!subpass->inputAttachments.empty())
		{
			description.inputAttachmentCount =
				static_cast<uint32_t>(subpass->inputAttachments.size());
			description.pInputAttachments = addAttachmentReferences(
				subpass->inputAttachments);
		}

		if (!subpass->colorAttachments.empty())
		{
			description.colorAttachmentCount =
				static_cast<uint32_t>(subpass->colorAttachments.size());
			description.pColorAttachments = addAttachmentReferences(
				subpass->colorAttachments,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}

		if (!subpass->resolveAttachments.empty())
		{
			description.pResolveAttachments = addAttachmentReferences(
				subpass->resolveAttachments,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		}

		if (subpass->depthAttachment != nullptr)
		{
			description.pDepthStencilAttachment = addAttachmentReference(
				subpass->depthAttachment,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		}

		subpassDescriptions.push_back(description);
	}

	for (auto i = 0; i < subpasses.size(); i++)
	{
		for (auto& dependency : subpasses[i]->dependencies)
		{
			VkSubpassDependency dep = {};
			dep.srcSubpass = static_cast<uint32_t>(i);

			bool found = false;
			auto j = i;
			for (; j < subpasses.size(); j++)
			{
				if (subpasses[j] == dependency.first)
				{
					found = true;
					break;
				}
			}
			if (!found) throw runtime_error("Dependency subpass not found.");

			dep.dstSubpass = static_cast<uint32_t>(j);
			dep.srcStageMask = dependency.second.srcStageMask;
			dep.dstStageMask = dependency.second.dstStageMask;
			dep.srcAccessMask = dependency.second.srcAccessMask;
			dep.dstAccessMask = dependency.second.dstAccessMask;
			dep.dependencyFlags = dependency.second.dependencyFlags;

			subpassDependencies.push_back(dep);
		}
	}

	create();
}

void RenderPass::render(Framebuffer& framebuffer, VkCommandBuffer cmdBuffer)
{
	framebuffer.before(cmdBuffer);

	auto clearValues = framebuffer.getClearValues();

	VkRenderPassBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = handle;
	beginInfo.framebuffer = framebuffer;
	beginInfo.renderArea = renderArea;
	beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	beginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(cmdBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

	subpasses[0]->render(renderArea, cmdBuffer);

	for (auto i = 1; i < subpasses.size(); i++)
	{
		vkCmdNextSubpass(cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
		subpasses[i]->render(renderArea, cmdBuffer);
	}

	vkCmdEndRenderPass(cmdBuffer);

	framebuffer.after(cmdBuffer);
}

uint32_t RenderPass::getAttachmentIndex(const AttachmentSlot* a)
{
	auto found = lower_bound(attachmentSlots.begin(), attachmentSlots.end(), a);
	if (found == attachmentSlots.end() || *found != a)
		throw runtime_error("No such attachment slot.");
	return static_cast<uint32_t>(distance(attachmentSlots.begin(), found));
}

void RenderPass::addAttachment(const AttachmentSlot* a)
{
	auto pos = lower_bound(attachmentSlots.begin(), attachmentSlots.end(), a);
	if (pos == attachmentSlots.end() || *pos != a)
	{
		attachmentSlots.insert(pos, a);
	}
}

const VkAttachmentReference* RenderPass::addAttachmentReference(const AttachmentSlot* attachment,
								VkImageLayout layout)
{
	auto index = attachmentReferences.size();
	attachmentReferences.emplace_back();
	attachmentReferences[index].push_back({getAttachmentIndex(attachment), layout});
	return attachmentReferences[index].data();
}

const VkAttachmentReference* RenderPass::addAttachmentReferences(
	vector<const AttachmentSlot*>& attachments, VkImageLayout layout)
{
	auto index = attachmentReferences.size();
	attachmentReferences.emplace_back();
	for (const AttachmentSlot* attachment : attachments)
		attachmentReferences[index].push_back({getAttachmentIndex(attachment), layout});
	return attachmentReferences[index].data();
}

const VkAttachmentReference* RenderPass::addAttachmentReferences(
	vector<pair<const AttachmentSlot*, VkImageLayout>>& attachments)
{
	auto index = attachmentReferences.size();
	attachmentReferences.emplace_back();
	for (auto& attachment : attachments)
		attachmentReferences[index].push_back(
			{getAttachmentIndex(attachment.first), attachment.second});
	return attachmentReferences[index].data();
}

void RenderPass::create()
{
	VkRenderPassCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
	info.pAttachments = attachmentDescriptions.data();
	info.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
	info.pSubpasses = subpassDescriptions.data();
	info.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	info.pDependencies = subpassDependencies.data();

	VkResult result = vkCreateRenderPass(*device, &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create render pass.");
}

}