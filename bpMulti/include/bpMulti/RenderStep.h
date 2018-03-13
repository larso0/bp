#ifndef BP_RENDERSTEP_H
#define BP_RENDERSTEP_H

#include "PipelineStep.h"
#include <bp/Device.h>
#include <bp/CommandPool.h>
#include <bp/Renderer.h>
#include <bp/OffscreenFramebuffer.h>

namespace bpMulti
{

template <class DerivedRenderer>
class RenderStep : public PipelineStep<bp::OffscreenFramebuffer>
{
	static_assert(std::is_base_of<bp::Renderer, DerivedRenderer>::value,
		      "DerivedRenderer must derive bp::Renderer.");
public:
	RenderStep() :
		device{nullptr},
		queue{nullptr},
		cmdBuffer{VK_NULL_HANDLE},
		width{0}, height{0},
		renderer{nullptr} {}
	virtual ~RenderStep() = default;

	void init(bp::Device& device, unsigned outputCount, uint32_t width, uint32_t height,
		  DerivedRenderer& renderer)
	{
		RenderStep::device = &device;
		RenderStep::width = width;
		RenderStep::height = height;
		RenderStep::renderer = &renderer;

		queue = &device.getGraphicsQueue();
		cmdPool.init(*queue);
		cmdBuffer = cmdPool.allocateCommandBuffer();

		renderer.init(device, VK_FORMAT_R8G8B8A8_UNORM, width, height);

		PipelineStep<bp::OffscreenFramebuffer>::init(outputCount);
	}

protected:
	bp::Device* device;
	bp::Queue* queue;
	bp::CommandPool cmdPool;
	VkCommandBuffer cmdBuffer;

	uint32_t width, height;
	DerivedRenderer* renderer;

	void prepare(bp::OffscreenFramebuffer& output) override
	{
		output.init(renderer->getRenderPass(), width, height,
			    renderer->getColorAttachmentSlot(), renderer->getDepthAttachmentSlot());
	}
};

}

#endif