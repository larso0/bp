#include <bp/Renderer.h>
#include <stdexcept>

using namespace std;

namespace bp
{

Renderer::Renderer(RenderPass& renderPass) :
	renderPass{renderPass}
{
	Device& device = renderPass.getRenderTarget().getDevice();
	VkCommandPool pool = renderPass.getRenderTarget().getCmdPool();

	VkCommandBufferAllocateInfo cmdBufferInfo = {};
	cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferInfo.commandPool = pool;
	cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferInfo.commandBufferCount = 1;
	VkResult result = vkAllocateCommandBuffers(device, &cmdBufferInfo, &cmdBuffer);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to allocate command buffer.");

	VkSemaphoreCreateInfo semInfo = {};
	semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	result = vkCreateSemaphore(device, &semInfo, nullptr, &renderCompleteSem);
	if (result != VK_SUCCESS)
		throw runtime_error("Failed to create semaphore.");
}

Renderer::~Renderer()
{
	Device& device = renderPass.getRenderTarget().getDevice();
	VkCommandPool pool = renderPass.getRenderTarget().getCmdPool();

	vkDestroySemaphore(device, renderCompleteSem, nullptr);
	vkFreeCommandBuffers(device, pool, 1, &cmdBuffer);
}

void Renderer::render(VkSemaphore waitSem)
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmdBuffer, &beginInfo);

	renderPass.getRenderTarget().beginFrame(cmdBuffer);
	renderPass.begin(cmdBuffer);
	draw(cmdBuffer);
	renderPass.end(cmdBuffer);
	renderPass.getRenderTarget().endFrame(cmdBuffer);

	vkEndCommandBuffer(cmdBuffer);

	VkPipelineStageFlags waitStages = {VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT};
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderCompleteSem;

	if (waitSem != VK_NULL_HANDLE)
	{
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &waitSem;
	}
	
	submitInfo.pWaitDstStageMask = &waitStages;

	Queue& queue = renderPass.getRenderTarget().getDevice().getGraphicsQueue();
	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);
}

}