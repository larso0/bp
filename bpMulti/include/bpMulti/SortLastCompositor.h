#ifndef BP_SORTLASTCOMPOSITOR_H
#define BP_SORTLASTCOMPOSITOR_H

#include "Compositor.h"
#include "SortLastRenderer.h"
#include <utility>

namespace bpMulti
{

class SortLastCompositor : public Compositor
{
public:
	void init(std::vector<std::pair<bp::Device*, SortLastRenderer*>> configurations,
		  VkFormat colorFormat, uint32_t width, uint32_t height);
	void resize(uint32_t width, uint32_t height) override;
private:
	VkExtent2D getContributionSize(unsigned deviceIndex) override;
	unsigned getCompositingElementCount() const override { return 3; }
	bool shouldCopyDepth() const override { return true; }
	void initShaders() override;
	void initDescriptorSetLayout() override;
	void initPipelineLayout() override;
	void initPipeline() override;
	void initDescriptorPool() override;
	void setupContribution(Contribution& contribution) override;
};

}

#endif
