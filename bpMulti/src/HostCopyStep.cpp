#include <bpMulti/HostCopyStep.h>
#include <bp/Util.h>
#include <future>

using namespace bp;
using namespace std;

namespace bpMulti
{

void HostCopyStep::resize(uint32_t width, uint32_t height)
{
	HostCopyStep::width = width;
	HostCopyStep::height = height;

	for (unsigned i = 0; i < getOutputCount(); i++)
	{
		destroy(getOutput(i));
		prepare(getOutput(i));
	}
}

void HostCopyStep::prepare(BufferPair& output)
{
	VkDeviceSize colorSize = width * height * 4;
	output.first = new Buffer(*outputDevice, colorSize,
				  VK_BUFFER_USAGE_TRANSFER_SRC_BIT
				  | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				  VMA_MEMORY_USAGE_CPU_ONLY);
	if (copyDepth)
	{
		VkDeviceSize depthSize = width * height * 2;
		output.second = new Buffer(*outputDevice, depthSize,
					   VK_BUFFER_USAGE_TRANSFER_SRC_BIT
					   | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					   VMA_MEMORY_USAGE_CPU_ONLY);
	}
}

void HostCopyStep::destroy(BufferPair& output)
{
	delete output.first;
	if (copyDepth) delete output.second;
}

void HostCopyStep::process(unsigned, BufferPair& output, BufferPair& input)
{
	future<void> colorCopyFuture = async(launch::async, [&]{
		size_t colorSize = width * height * 4;
		parallelCopy(output.first->map(), input.first->map(), colorSize);
	});
	
	if (copyDepth)
	{
		size_t depthSize = width * height * 2;
		parallelCopy(output.second->map(), input.second->map(), depthSize);
	}

	colorCopyFuture.wait();
}

}