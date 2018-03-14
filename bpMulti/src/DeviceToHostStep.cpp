#include <bpMulti/DeviceToHostStep.h>

using namespace bp;

namespace bpMulti
{

void DeviceToHostStep::process(OffscreenFramebuffer& input, BufferPair& output, unsigned)
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