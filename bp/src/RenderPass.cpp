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
		destroyFramebuffers();
		vkDestroyRenderPass(*device, handle, nullptr);
	}
}

void RenderPass::addSubpassGraph(Subpass& subpass)
{
	if (device == nullptr) device = subpass.device;
	subpasses.push_back(&subpass);

	addAttachments(subpass.inputAttachments.begin(), subpass.inputAttachments.end());
	addAttachments(subpass.colorAttachments.begin(), subpass.colorAttachments.end());
	addAttachments(subpass.resolveAttachments.begin(), subpass.resolveAttachments.end());
	if (subpass.depthAttachment != nullptr) addAttachment(subpass.depthAttachment);

	for (auto s : subpass.dependents)
	{
		addSubpassGraph(*s);
	}
}

void RenderPass::init(uint32_t width, uint32_t height)
{
	renderExtent.width = width;
	renderExtent.height = height;

	if (subpasses.empty())
		throw runtime_error("No subpasses added to render pass.");

	for (Attachment* attachment : attachments)
	{
		VkAttachmentDescription description = {};
		description.format = attachment->getFormat();
		description.samples = VK_SAMPLE_COUNT_1_BIT;

		if (attachment->isClearEnabled()) description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		else description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

		description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		description.initialLayout = attachment->getInitialLayout();
		description.finalLayout = attachment->getFinalLayout();

		attachmentDescriptions.push_back(description);
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
				subpass->inputAttachments,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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

	for (Subpass* subpass : subpasses) subpass->init(*this);

	if (swapchain != nullptr)
	{
		bpUtil::connect(swapchain->resizeEvent, *this, &RenderPass::resize);
	}
}

void RenderPass::resize(uint32_t width, uint32_t height)
{
	renderExtent.width = width;
	renderExtent.height = height;
	destroyFramebuffers();
	createFramebuffers();
}

void RenderPass::render(VkCommandBuffer cmdBuffer)
{
	for (Attachment* attachment : attachments) attachment->before(cmdBuffer);

	vector<VkClearValue> clearValues(attachments.size());
	auto framebufferIndex = swapchain != nullptr ? swapchain->getCurrentFramebufferIndex() : 0;

	VkRenderPassBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = handle;
	beginInfo.framebuffer = framebuffers[framebufferIndex];
	beginInfo.renderArea = renderArea;

	for (auto i = 0; i < attachments.size(); i++)
	{
		const auto& description = attachmentDescriptions[i];
		if (description.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR
		    || description.stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
		{
			clearValues[i] = attachments[i]->getClearValue();
		}
	}

	beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	beginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(cmdBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

	subpasses[0]->render(cmdBuffer);

	for (auto i = 1; i < subpasses.size(); i++)
	{
		vkCmdNextSubpass(cmdBuffer, VK_SUBPASS_CONTENTS_INLINE);
		subpasses[i]->render(cmdBuffer);
	}

	vkCmdEndRenderPass(cmdBuffer);

	for (Attachment* attachment : attachments) attachment->after(cmdBuffer);
}

uint32_t RenderPass::getAttachmentIndex(Attachment* a)
{
	auto found = lower_bound(attachments.begin(), attachments.end(), a);
	if (found == attachments.end() || *found != a)
		throw runtime_error("No such attachment.");
	return static_cast<uint32_t>(distance(attachments.begin(), found));
}

void RenderPass::addAttachment(Attachment* a)
{
	auto pos = lower_bound(attachments.begin(), attachments.end(), a);
	if (pos == attachments.end() || *pos != a)
	{
		attachments.insert(pos, a);
	}
	auto swapchainAttachment = dynamic_cast<Swapchain*>(a);
	if (swapchainAttachment != nullptr)
	{
		if (swapchain != nullptr) throw runtime_error(
			"Can only use one swapchain attachment per render pass.");
		swapchain = swapchainAttachment;
	}
}

const VkAttachmentReference* RenderPass::addAttachmentReference(Attachment* attachment,
								VkImageLayout layout)
{
	auto index = attachmentReferences.size();
	attachmentReferences.emplace_back();
	attachmentReferences[index].push_back({getAttachmentIndex(attachment), layout});
	return attachmentReferences[index].data();
}

const VkAttachmentReference* RenderPass::addAttachmentReferences(
	std::vector<Attachment*>& attachments, VkImageLayout layout)
{
	auto index = attachmentReferences.size();
	attachmentReferences.emplace_back();
	for (Attachment* attachment : attachments)
		attachmentReferences[index].push_back({getAttachmentIndex(attachment), layout});
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

	createFramebuffers();
}

void RenderPass::createFramebuffers()
{
	vector<VkImageView> attachmentViews;
	int swapchainIndex = -1;
	for (int i = 0; i < attachments.size(); i++)
	{
		if (swapchain != nullptr && attachments[i] == static_cast<Attachment*>(swapchain))
		{
			swapchainIndex = i;
			attachmentViews.push_back(VK_NULL_HANDLE);
			continue;
		}
		auto texture = dynamic_cast<Texture*>(attachments[i]);
		if (texture == nullptr)
			throw runtime_error("Unsupported attachment subclass.");
		attachmentViews.push_back(texture->getImageView());
	}
	if (swapchain != nullptr && swapchainIndex == -1)
		throw runtime_error("This should never happen.");

	VkFramebufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.renderPass = handle;
	info.attachmentCount = static_cast<uint32_t>(attachmentViews.size());
	info.pAttachments = attachmentViews.data();
	info.width = renderExtent.width;
	info.height = renderExtent.height;
	info.layers = 1;

	auto framebufferCount = swapchain != nullptr ? swapchain->getFramebufferImageCount() : 1;
	framebuffers.resize(framebufferCount);
	for (auto i = 0; i < framebufferCount; i++)
	{
		if (swapchain != nullptr)
		{
			attachmentViews[swapchainIndex] =
				swapchain->getFramebufferImageView(static_cast<uint32_t>(i));
		}
		VkResult result = vkCreateFramebuffer(*device, &info, nullptr,
						      framebuffers.data() + i);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create framebuffer.");
	}
}

void RenderPass::destroyFramebuffers()
{
	for (VkFramebuffer fb : framebuffers)
		vkDestroyFramebuffer(*device, fb, nullptr);
}

}