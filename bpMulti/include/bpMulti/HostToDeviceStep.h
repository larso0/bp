#ifndef BP_HOSTTODEVICESTEP_H
#define BP_HOSTTODEVICESTEP_H

#include "PipelineStep.h"
#include <bp/Buffer.h>
#include <bp/Texture.h>
#include <utility>
#include <vector>

namespace bpMulti
{

using BufferPair = std::pair<bp::Buffer*, bp::Buffer*>;
using TexturePair = std::pair<bp::Texture*, bp::Texture*>;

class HostToDeviceStep :
	public PipelineStep<std::vector<TexturePair>, std::vector<BufferPair>&, VkCommandBuffer>
{
public:
	HostToDeviceStep() :
		outputDevice{nullptr},
		copyDepth{true},
		width{0}, height{0},
		deviceCount{1} {}

	void init(bp::Device& outputDevice, bool copyDepth, uint32_t width, uint32_t height,
		  unsigned outputCount, unsigned deviceCount);
	void resize(uint32_t width, uint32_t height);

private:
	bp::Device* outputDevice;
	bool copyDepth;
	uint32_t width, height;
	unsigned deviceCount;

	void prepare(std::vector<TexturePair>& output) override;
	void destroy(std::vector<TexturePair>& output) override;
	void process(unsigned, std::vector<TexturePair>& output,
		     std::vector<BufferPair>& input, VkCommandBuffer cmdBuffer) override;
};

}

#endif