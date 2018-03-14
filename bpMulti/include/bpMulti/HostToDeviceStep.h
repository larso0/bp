#ifndef BP_HOSTTODEVICESTEP_H
#define BP_HOSTTODEVICESTEP_H

#include "PipelineStep.h"
#include <bp/Buffer.h>
#include <bp/Texture.h>
#include <utility>

namespace bpMulti
{

using BufferPair = std::pair<bp::Buffer*, bp::Buffer*>;
using TexturePair = std::pair<bp::Texture*, bp::Texture*>;

class HostToDeviceStep : public PipelineStep<TexturePair, BufferPair&>
{
public:
	HostToDeviceStep() :
		outputDevice{nullptr} {}

	void init(bp::Device& outputDevice, bool copyDepth, uint32_t width, uint32_t height,
		  unsigned outputCount)
	{
		HostToDeviceStep::outputDevice = &outputDevice;
		HostToDeviceStep::copyDepth = copyDepth;
		HostToDeviceStep::width = width;
		HostToDeviceStep::height = height;

		PipelineStep<TexturePair, BufferPair&>::init(outputCount);
	}
	void resize(uint32_t width, uint32_t height);

private:
	bp::Device* outputDevice;
	bool copyDepth;
	uint32_t width, height;

	void prepare(TexturePair& output) override;
	void destroy(TexturePair& output) override;
	void process(BufferPair& input, TexturePair& output, unsigned) override;
};

}

#endif