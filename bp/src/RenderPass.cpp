#include <bp/RenderPass.h>
#include <stdexcept>

using namespace std;

namespace bp
{

void RenderPass::init(NotNull<RenderTarget> target, const VkRect2D& area)
{
	renderTarget = target;
	renderArea = area;
	create();
}

RenderPass::~RenderPass()
{
	for (VkFramebuffer fb : framebuffers)
		vkDestroyFramebuffer(renderTarget->getDevice(), fb, nullptr);
	vkDestroyRenderPass(renderTarget->getDevice(), handle, nullptr);
}

void RenderPass::recreateFramebuffers()
{
	for (VkFramebuffer fb : framebuffers)
		vkDestroyFramebuffer(renderTarget->getDevice(), fb, nullptr);
	createFramebuffers();
}

void RenderPass::begin(VkCommandBuffer cmdBuffer)
{
	vector<VkClearValue> clearValues;
	if (clearEnabled)
	{
		clearValues.push_back(clearValue);
		if (renderTarget->isDepthImageEnabled()) clearValues.push_back({1.f, 0.f});
	} else if (renderTarget->isDepthImageEnabled())
	{
		clearValues.push_back(clearValue);
		clearValues.push_back({1.f, 0.f});
	}
	VkRenderPassBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = handle;
	beginInfo.framebuffer = framebuffers[renderTarget->getCurrentFramebufferIndex()];
	beginInfo.renderArea = renderArea;

	beginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	beginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(cmdBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::end(VkCommandBuffer cmdBuffer)
{
	vkCmdEndRenderPass(cmdBuffer);
}

void RenderPass::setRenderArea(VkRect2D area)
{
	renderArea = area;
}

void RenderPass::setClearEnabled(bool enabled)
{
	clearEnabled = enabled;
}

void RenderPass::setClearValue(const VkClearValue& value)
{
	clearValue = value;
}

void RenderPass::create()
{
	uint32_t attachmentCount = 1;
	VkAttachmentDescription passAttachments[2] = {};
	passAttachments[0].format = renderTarget->getFormat();
	passAttachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	passAttachments[0].loadOp =  clearEnabled ? VK_ATTACHMENT_LOAD_OP_CLEAR
						  : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	passAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	passAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	passAttachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	passAttachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	passAttachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	if (renderTarget->isDepthImageEnabled())
	{
		passAttachments[1].format = VK_FORMAT_D16_UNORM;
		passAttachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		passAttachments[1].loadOp =  VK_ATTACHMENT_LOAD_OP_CLEAR;
		passAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		passAttachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		passAttachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		passAttachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		passAttachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachmentCount++;
	}

	VkAttachmentReference colorAttachment = {};
	colorAttachment.attachment = 0;
	colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachment = {};
	depthAttachment.attachment = 1;
	depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachment;

	if (renderTarget->isDepthImageEnabled())
		subpass.pDepthStencilAttachment = &depthAttachment;

	VkRenderPassCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = attachmentCount;
	info.pAttachments = passAttachments;
	info.subpassCount = 1;
	info.pSubpasses = &subpass;

	VkResult result = vkCreateRenderPass(renderTarget->getDevice(), &info, nullptr, &handle);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create render pass.");

	createFramebuffers();
}

void RenderPass::createFramebuffers()
{
	VkImageView attachments[2];
	attachments[1] = renderTarget->getDepthImageView();

	VkFramebufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.renderPass = handle;
	info.attachmentCount = renderTarget->isDepthImageEnabled() ? 2 : 1;
	info.pAttachments = attachments;
	info.width = renderTarget->getWidth();
	info.height = renderTarget->getHeight();
	info.layers = 1;

	uint32_t n = renderTarget->getFramebufferImageCount();
	framebuffers.resize(n);
	for (uint32_t i = 0; i < n; i++)
	{
		attachments[0] = renderTarget->getFramebufferImageViews()[i];
		VkResult result = vkCreateFramebuffer(renderTarget->getDevice(), &info, nullptr,
						      framebuffers.data() + i);
		if (result != VK_SUCCESS)
			throw runtime_error("Failed to create framebuffer.");
	}
}

}