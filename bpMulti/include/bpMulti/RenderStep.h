#ifndef BP_RENDERSTEP_H
#define BP_RENDERSTEP_H

#include "PipelineStep.h"
#include <bp/Device.h>
#include <bp/CommandPool.h>
#include <bp/Renderer.h>
#include <bp/OffscreenFramebuffer.h>

namespace bpMulti
{

class RenderStep : public PipelineStep<bp::OffscreenFramebuffer>
{
public:
	RenderStep() :
		device{nullptr},
		queue{nullptr},
		cmdBuffer{VK_NULL_HANDLE},
		width{0}, height{0},
		renderer{nullptr} {}
	virtual ~RenderStep() = default;

	void init(bp::Device& device, unsigned outputCount, uint32_t width, uint32_t height,
		  bp::Renderer& renderer);
	void resize(uint32_t width, uint32_t height);
	bp::Renderer& getRenderer() { return *renderer; }

protected:
	bp::Device* device;
	bp::Queue* queue;
	bp::CommandPool cmdPool;
	VkCommandBuffer cmdBuffer;

	uint32_t width, height;
	bp::Renderer* renderer;

	void prepare(bp::OffscreenFramebuffer& output) override;
	void process(bp::OffscreenFramebuffer& output, unsigned) override;
};

}

#endif