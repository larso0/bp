#include <bpMulti/RenderStep.h>

using namespace bp;

namespace bpMulti
{

void RenderStep::init(Device& device, unsigned outputCount, uint32_t width, uint32_t height,
		      Renderer& renderer)
{
	RenderStep::device = &device;
	RenderStep::width = width;
	RenderStep::height = height;
	RenderStep::renderer = &renderer;

	queue = &device.getGraphicsQueue();
	cmdPool.init(*queue);
	cmdBuffer = cmdPool.allocateCommandBuffer();

	renderer.init(device, VK_FORMAT_R8G8B8A8_UNORM, width, height);

	PipelineStep<OffscreenFramebuffer>::init(outputCount);
}

void RenderStep::resize(uint32_t width, uint32_t height)
{
	RenderStep::width = width;
	RenderStep::height = height;

	for (unsigned i = 0; i < getOutputCount(); i++)
	{
		getOutput(i).resize(width, height);
	}
	renderer->resize(width, height);
}

void RenderStep::prepare(bp::OffscreenFramebuffer& output)
{
	output.init(renderer->getRenderPass(), width, height,
		    renderer->getColorAttachmentSlot(), renderer->getDepthAttachmentSlot());
}

void RenderStep::process(unsigned, bp::OffscreenFramebuffer& output)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(cmdBuffer, &beginInfo);

	renderer->render(output, cmdBuffer);

	vkEndCommandBuffer(cmdBuffer);
	queue->submit({}, {cmdBuffer}, {});
	queue->waitIdle();
}

}