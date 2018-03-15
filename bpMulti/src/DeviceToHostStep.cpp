#include <bpMulti/DeviceToHostStep.h>

using namespace bp;

namespace bpMulti
{

void DeviceToHostStep::process(unsigned, BufferPair& output, bp::OffscreenFramebuffer& input)
{
	input.getColorAttachment().getImage().updateStagingBuffer();
	output.first = input.getColorAttachment().getImage().getStagingBuffer();
	if (copyDepth)
	{
		input.getDepthAttachment().getImage().updateStagingBuffer();
		output.second = input.getDepthAttachment().getImage().getStagingBuffer();
	}
}

}