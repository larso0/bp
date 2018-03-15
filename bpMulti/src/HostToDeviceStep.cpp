#include <bpMulti/HostToDeviceStep.h>

using namespace bp;
using namespace std;

namespace bpMulti
{

void HostToDeviceStep::init(bp::Device& outputDevice, bool copyDepth, uint32_t width,
			    uint32_t height, unsigned outputCount, unsigned deviceCount)
{
	HostToDeviceStep::outputDevice = &outputDevice;
	HostToDeviceStep::copyDepth = copyDepth;
	HostToDeviceStep::width = width;
	HostToDeviceStep::height = height;
	HostToDeviceStep::deviceCount = deviceCount;

	PipelineStep<vector<TexturePair>, vector<BufferPair>&, VkCommandBuffer>::init(outputCount);
}

void HostToDeviceStep::resize(uint32_t width, uint32_t height)
{
	HostToDeviceStep::width = width;
	HostToDeviceStep::height = height;

	for (unsigned i = 0; i < getOutputCount(); i++)
	{
		for (auto& p : getOutput(i))
		{
			p.first->resize(width, height);
			if (copyDepth) p.second->resize(width, height);
		}
	}
}

void HostToDeviceStep::prepare(vector<TexturePair>& output)
{
	output.resize(deviceCount);

	for (auto& p : output)
	{
		p.first = new Texture(*outputDevice, VK_FORMAT_R8G8B8A8_UNORM,
				      VK_IMAGE_USAGE_SAMPLED_BIT, width, height);
		if (copyDepth)
		{
			p.second = new Texture(*outputDevice, VK_FORMAT_D16_UNORM,
					       VK_IMAGE_USAGE_SAMPLED_BIT, width, height);
		}
	}
}

void HostToDeviceStep::destroy(vector<TexturePair>& output)
{
	for (auto& p : output)
	{
		delete p.first;
		if (copyDepth) delete p.second;
	}
}

void HostToDeviceStep::process(unsigned, vector<TexturePair>& output, vector<BufferPair>& input,
			       VkCommandBuffer cmdBuffer)
{
	for (unsigned i = 0; i < deviceCount; i++)
	{
		auto& in = input[i];
		auto& out = output[i];

		out.first->getImage().transfer(*in.first, cmdBuffer);
		out.first->transitionShaderReadable(cmdBuffer,
						    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		if (copyDepth)
		{
			out.second->getImage().transfer(*in.second, cmdBuffer);
			out.second->transitionShaderReadable(cmdBuffer,
							     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		}
	}
}

}