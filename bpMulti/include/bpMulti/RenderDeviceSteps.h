#ifndef BP_RENDERDEVICESTEPS_H
#define BP_RENDERDEVICESTEPS_H

#include <bp/CommandPool.h>
#include <bp/OffscreenFramebuffer.h>
#include <bp/Renderer.h>

namespace bpMulti
{

class RenderDeviceSteps
{
public:
	RenderDeviceSteps() :
		device{nullptr},
		graphicsQueue{nullptr},
		transferQueue{nullptr},
		renderCmdBuffer{VK_NULL_HANDLE}, transferCmdBuffer{VK_NULL_HANDLE},
		renderer{nullptr} {}

	void init(bp::Device& device, bp::Renderer& renderer, uint32_t width, uint32_t height,
		  unsigned framebufferCount = 1);
	void resize(uint32_t width, uint32_t height);
	void render(unsigned framebufferIndex);
	void deviceToHost(unsigned framebufferIndex, bool copyDepth = true);
	bp::OffscreenFramebuffer& getFramebuffer(unsigned index) { return framebuffers[index]; }

private:
	bp::Device* device;
	bp::Queue* graphicsQueue;
	bp::Queue* transferQueue;
	bp::CommandPool graphicsCmdPool, transferCmdPool;
	VkCommandBuffer renderCmdBuffer, transferCmdBuffer;

	bp::Renderer* renderer;
	std::vector<bp::OffscreenFramebuffer> framebuffers;
};

}

#endif