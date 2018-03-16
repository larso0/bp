#include <bpMulti/RenderDeviceSteps.h>

using namespace bp;

namespace bpMulti
{

void RenderDeviceSteps::init(Device& device, Renderer& renderer, uint32_t width, uint32_t height,
		      unsigned framebufferCount)
{
	RenderDeviceSteps::device = &device;
	RenderDeviceSteps::renderer = &renderer;

	graphicsQueue = &device.getGraphicsQueue();
	transferQueue = &device.getTransferQueue();
	graphicsCmdPool.init(*graphicsQueue);
	transferCmdPool.init(*transferQueue);
	renderCmdBuffer = graphicsCmdPool.allocateCommandBuffer();
	transferCmdBuffer = transferCmdPool.allocateCommandBuffer();

	renderer.init(device, VK_FORMAT_R8G8B8A8_UNORM, width, height);
	framebuffers.resize(framebufferCount);
	for (auto& fb : framebuffers)
	{
		fb.init(renderer.getRenderPass(), width, height, renderer.getColorAttachmentSlot(),
			renderer.getDepthAttachmentSlot());
	}
}

void RenderDeviceSteps::render(unsigned framebufferIndex)
{
	VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
					      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};
	vkBeginCommandBuffer(renderCmdBuffer, &beginInfo);

	renderer->render(framebuffers[framebufferIndex], renderCmdBuffer);

	vkEndCommandBuffer(renderCmdBuffer);
	graphicsQueue->submit({}, {renderCmdBuffer}, {});
	graphicsQueue->waitIdle();
}

void RenderDeviceSteps::deviceToHost(unsigned framebufferIndex, bool copyDepth)
{
	VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
					      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr};
	vkBeginCommandBuffer(transferCmdBuffer, &beginInfo);

	auto& fb = framebuffers[framebufferIndex];
	fb.getColorAttachment().getImage().updateStagingBuffer(transferCmdBuffer);
	if (copyDepth) fb.getDepthAttachment().getImage().updateStagingBuffer(transferCmdBuffer);

	vkEndCommandBuffer(transferCmdBuffer);
	transferQueue->submit({}, {transferCmdBuffer}, {});
	transferQueue->waitIdle();
}

}