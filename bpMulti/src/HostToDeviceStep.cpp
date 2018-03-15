#include <bpMulti/HostToDeviceStep.h>

using namespace bp;
using namespace std;

namespace bpMulti
{

void HostToDeviceStep::init(Device& outputDevice, DescriptorSetLayout& descriptorSetLayout,
			    DescriptorPool& descriptorPool, bool copyDepth, uint32_t width,
			    uint32_t height, unsigned outputCount, unsigned deviceCount)
{
	HostToDeviceStep::outputDevice = &outputDevice;
	HostToDeviceStep::descriptorSetLayout = &descriptorSetLayout;
	HostToDeviceStep::descriptorPool = &descriptorPool;
	HostToDeviceStep::copyDepth = copyDepth;
	HostToDeviceStep::width = width;
	HostToDeviceStep::height = height;
	HostToDeviceStep::deviceCount = deviceCount;

	PipelineStep<vector<Contribution>, vector<BufferPair>&, VkCommandBuffer>::init(outputCount);
}

void HostToDeviceStep::resize(uint32_t width, uint32_t height)
{
	HostToDeviceStep::width = width;
	HostToDeviceStep::height = height;

	for (unsigned i = 0; i < getOutputCount(); i++)
	{
		for (auto& c : getOutput(i))
		{
			c.colorTexture.resize(width, height);
			if (copyDepth) c.depthTexture->resize(width, height);
			c.descriptorSet.update();
		}
	}
}

void HostToDeviceStep::prepare(vector<Contribution>& output)
{
	output.resize(deviceCount);

	for (auto& c : output)
	{
		c.colorTexture.init(*outputDevice, VK_FORMAT_R8G8B8A8_UNORM,
				    VK_IMAGE_USAGE_SAMPLED_BIT, width, height);
		c.descriptorSet.init(*outputDevice, *descriptorPool, *descriptorSetLayout);
		c.descriptorSet.bind(c.colorTexture.getDescriptor());
		if (copyDepth)
		{
			c.depthTexture = new Texture(*outputDevice, VK_FORMAT_D16_UNORM,
						     VK_IMAGE_USAGE_SAMPLED_BIT, width, height);
			c.descriptorSet.bind(c.depthTexture->getDescriptor());
		}
		c.descriptorSet.update();
	}
}

void HostToDeviceStep::destroy(vector<Contribution>& output)
{
	for (auto& c : output)
		if (copyDepth) delete c.depthTexture;
}

void HostToDeviceStep::process(unsigned, vector<Contribution>& output, vector<BufferPair>& input,
			       VkCommandBuffer cmdBuffer)
{
	for (unsigned i = 0; i < deviceCount; i++)
	{
		auto& in = input[i];
		auto& out = output[i];

		out.colorTexture.getImage().transfer(*in.first, cmdBuffer);
		out.colorTexture.transitionShaderReadable(cmdBuffer,
							  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		if (copyDepth)
		{
			out.depthTexture->getImage().transfer(*in.second, cmdBuffer);
			out.depthTexture->transitionShaderReadable(
				cmdBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		}
	}
}

}