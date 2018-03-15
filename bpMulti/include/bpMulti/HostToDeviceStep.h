#ifndef BP_HOSTTODEVICESTEP_H
#define BP_HOSTTODEVICESTEP_H

#include "PipelineStep.h"
#include <bp/Buffer.h>
#include <bp/Texture.h>
#include <bp/DescriptorSetLayout.h>
#include <bp/DescriptorPool.h>
#include <bp/DescriptorSet.h>
#include <utility>
#include <vector>

namespace bpMulti
{

using BufferPair = std::pair<bp::Buffer*, bp::Buffer*>;

struct Contribution
{
	bp::Texture colorTexture;
	bp::Texture* depthTexture;
	bp::DescriptorSet descriptorSet;
};

class HostToDeviceStep :
	public PipelineStep<std::vector<Contribution>, std::vector<BufferPair>&, VkCommandBuffer>
{
public:
	HostToDeviceStep() :
		outputDevice{nullptr},
		descriptorSetLayout{nullptr},
		descriptorPool{nullptr},
		copyDepth{true},
		width{0}, height{0},
		deviceCount{1} {}

	void init(bp::Device& outputDevice, bp::DescriptorSetLayout& descriptorSetLayout,
		  bp::DescriptorPool& descriptorPool, bool copyDepth, uint32_t width,
		  uint32_t height, unsigned outputCount, unsigned deviceCount);
	void resize(uint32_t width, uint32_t height);

private:
	bp::Device* outputDevice;
	bp::DescriptorSetLayout* descriptorSetLayout;
	bp::DescriptorPool* descriptorPool;

	bool copyDepth;
	uint32_t width, height;
	unsigned deviceCount;

	void prepare(std::vector<Contribution>& output) override;
	void destroy(std::vector<Contribution>& output) override;
	void process(unsigned, std::vector<Contribution>& output,
		     std::vector<BufferPair>& input, VkCommandBuffer cmdBuffer) override;
};

}

#endif