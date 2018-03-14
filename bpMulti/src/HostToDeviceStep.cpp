#include <bpMulti/HostToDeviceStep.h>

using namespace bp;

namespace bpMulti
{

void HostToDeviceStep::resize(uint32_t width, uint32_t height)
{
	HostToDeviceStep::width = width;
	HostToDeviceStep::height = height;

	for (unsigned i = 0; i < getOutputCount(); i++)
	{
		auto& o = getOutput(i);
		o.first->resize(width, height);
		if (copyDepth) o.second->resize(width, height);
	}
}

void HostToDeviceStep::prepare(TexturePair& output)
{
	output.first = new Texture(*outputDevice, VK_FORMAT_R8G8B8A8_UNORM,
				   VK_IMAGE_USAGE_SAMPLED_BIT, width, height);
	if (copyDepth)
	{
		output.first = new Texture(*outputDevice, VK_FORMAT_D16_UNORM,
					   VK_IMAGE_USAGE_SAMPLED_BIT, width, height);
	}
}

void HostToDeviceStep::destroy(TexturePair& output)
{
	delete output.first;
	if (copyDepth) delete output.second;
}

void HostToDeviceStep::process(BufferPair& input, TexturePair& output, unsigned)
{
	output.first->getImage().transfer(*input.first);
	if (copyDepth) output.second->getImage().transfer(*input.second);
}

}