#include <bp/Renderer.h>
#include <stdexcept>

using namespace std;

namespace bp
{

void Renderer::init(Device& device, VkFormat colorFormat, uint32_t width, uint32_t height)
{
	Renderer::width = width;
	Renderer::height = height;
	Renderer::device = &device;

	colorAttachmentSlot.init(colorFormat, VK_SAMPLE_COUNT_1_BIT,
				 VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
				 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	depthAttachmentSlot.init(VK_FORMAT_D16_UNORM, VK_SAMPLE_COUNT_1_BIT,
				 VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
				 VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				 VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	setupSubpasses();

	renderPass.setRenderArea({{}, {width, height}});
	renderPass.init(device);

	initResources(width, height);
}

void Renderer::resize(uint32_t width, uint32_t height)
{
	Renderer::width = width;
	Renderer::height = height;
	renderPass.setRenderArea({{}, {width, height}});
}

void Renderer::render(Framebuffer& fbo, VkCommandBuffer cmdBuffer)
{
	renderPass.render(fbo, cmdBuffer);
}

void Renderer::addSubpassGraph(Subpass& subpass)
{
	if (isReady())
		throw runtime_error("Cannot add subpasses after initialization of renderer.");
	renderPass.addSubpassGraph(subpass);
}

}