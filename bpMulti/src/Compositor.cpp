#include <bpMulti/Compositor.h>
#include <future>
using namespace std;

namespace bpMulti
{

void Compositor::render(bp::Framebuffer& fbo, VkCommandBuffer cmdBuffer)
{
	unsigned nextFrameIndex = (currentFrameIndex + 1) % 2;

	auto primaryRenderFuture = async(launch::async, [this, nextFrameIndex]{
		primaryRenderDeviceSteps.render(nextFrameIndex);
	});

	vector<future<void>> renderFutures;
	for (auto& steps : secondaryRenderDeviceSteps)
	{
		renderFutures.push_back(async(launch::async, [&steps, this, nextFrameIndex]{
			steps.render(nextFrameIndex);
			steps.deviceToHost(nextFrameIndex, shouldCopyDepth());
		}));
	}
	hostCopyStep();
	primaryRenderFuture.wait();
	hostToDeviceStep();

	Renderer::render(fbo, cmdBuffer);
	for (auto& f : renderFutures) f.wait();
	currentFrameIndex = nextFrameIndex;
}

void Compositor::hostToDeviceStep()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(transferCommandBuffer, &beginInfo);

	primaryContributions[currentFrameIndex].flushStagingBuffers(transferCommandBuffer);
	for (auto& c : secondaryContributions) c.flushStagingBuffers(transferCommandBuffer);

	vkEndCommandBuffer(transferCommandBuffer);
	transferQueue->submit({}, {transferCommandBuffer}, {});
	transferQueue->waitIdle();
}

void Compositor::setupSubpasses()
{
	subpass.addColorAttachment(getColorAttachmentSlot());
	subpass.setDepthAttachment(getDepthAttachmentSlot());
	addSubpassGraph(subpass);
}

void Compositor::initResources(uint32_t width, uint32_t height)
{
	initShaders();
	initDescriptorSetLayout();
	initPipelineLayout();
	initPipeline();
	initDescriptorPool();

	transferQueue = &getDevice().getTransferQueue();
	transferCommandPool.init(*transferQueue);
	transferCommandBuffer = transferCommandPool.allocateCommandBuffer();

	drawables.resize(deviceCount);

	//Setup resources for primary device
	auto primarySize = getContributionSize(0);
	primaryRenderDeviceSteps.init(getDevice(), *primaryRenderer, primarySize.width,
				      primarySize.height, 2);
	primaryContributions.resize(2);
	for (auto& c : primaryContributions)
	{
		c.init(getDevice(), descriptorPool, descriptorSetLayout, pipelineLayout,
		       primarySize.width, primarySize.height);
		setupContribution(c);
		c.update();
	}

	drawables[0].init(pipeline, getCompositingElementCount());
	bpUtil::connect(drawables[0].resourceBindingEvent, [this](VkCommandBuffer cmdBuffer)
	{
		primaryContributions[currentFrameIndex].bind(cmdBuffer);
	});

	//Setup resources for secondary devices
	secondaryRenderDeviceSteps.resize(deviceCount - 1);
	secondaryContributions.resize(deviceCount - 1);
	for (unsigned i = 0; i < deviceCount - 1; i++)
	{
		auto size = getContributionSize(i + 1);
		secondaryRenderDeviceSteps[i].init(*secondaryDevices[i], *secondaryRenderers[i],
						   size.width, size.height, 2);
		secondaryContributions[i].init(getDevice(), descriptorPool, descriptorSetLayout,
					       pipelineLayout, size.width, size.height);
		setupContribution(secondaryContributions[i]);
		secondaryContributions[i].update();
		drawables[i + 1].init(pipeline, getCompositingElementCount());
		bpUtil::connect(drawables[i + 1].resourceBindingEvent, secondaryContributions[i],
				&Contribution::bind);
	}

	for (auto& d : drawables) subpass.addDrawable(d);
}

}