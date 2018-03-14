#ifndef BP_HOSTCOPYSTEP_H
#define BP_HOSTCOPYSTEP_H

#include "PipelineStep.h"
#include <bp/Buffer.h>
#include <utility>

namespace bpMulti
{

using BufferPair = std::pair<bp::Buffer*, bp::Buffer*>;

class HostCopyStep : public PipelineStep<BufferPair, BufferPair&>
{
public:
	HostCopyStep() :
		outputDevice{nullptr},
		copyDepth{true} {}

	void init(bp::Device& outputDevice, bool copyDepth, uint32_t width, uint32_t height,
		  unsigned outputCount)
	{
		HostCopyStep::outputDevice = &outputDevice;
		HostCopyStep::copyDepth = copyDepth;
		HostCopyStep::width = width;
		HostCopyStep::height = height;
		PipelineStep<BufferPair, BufferPair&>::init(outputCount);
	}

	void resize(uint32_t width, uint32_t height);

private:
	bp::Device* outputDevice;
	bool copyDepth;
	uint32_t width, height;

	void prepare(BufferPair& output) override;
	void destroy(BufferPair& output) override;
	void process(BufferPair& input, BufferPair& output, unsigned) override;
};

}

#endif