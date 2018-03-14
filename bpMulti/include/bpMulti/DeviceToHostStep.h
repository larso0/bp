#ifndef BP_DEVICETOHOSTSTEP_H
#define BP_DEVICETOHOSTSTEP_H

#include "PipelineStep.h"
#include <bp/OffscreenFramebuffer.h>
#include <bp/Buffer.h>
#include <utility>

namespace bpMulti
{

using BufferPair = std::pair<bp::Buffer*, bp::Buffer*>;

class DeviceToHostStep : public PipelineStep<BufferPair, bp::OffscreenFramebuffer&>
{
public:
	DeviceToHostStep() :
		copyDepth{true} {}

	void init(bool copyDepth, unsigned outputCount)
	{
		DeviceToHostStep::copyDepth = copyDepth;
		PipelineStep<BufferPair, bp::OffscreenFramebuffer&>::init(outputCount);
	}

private:
	bool copyDepth;
	void process(bp::OffscreenFramebuffer& input, BufferPair& output, unsigned) override;
};

}

#endif