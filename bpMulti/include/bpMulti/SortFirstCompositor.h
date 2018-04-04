#ifndef BP_SORTFIRSTCOMPOSITOR_H
#define BP_SORTFIRSTCOMPOSITOR_H

#include "Compositor.h"
#include "SortFirstRenderer.h"
#include <utility>

namespace bpMulti
{

class SortFirstCompositor : public Compositor
{
public:
	void init(std::vector<std::pair<bp::Device*, SortFirstRenderer*>> configurations,
		  VkFormat colorFormat, uint32_t width, uint32_t height);
	void resize(uint32_t width, uint32_t height) override;
private:
	std::vector<VkRect2D> contributionRegions;

	struct PushConstants
	{
		glm::vec2 offset, extent;
	};

	VkExtent2D getContributionSize(unsigned deviceIndex) override;
	unsigned int getCompositingElementCount() const override { return 4; }
	bool shouldCopyDepth() const override { return false; }
	void initShaders() override;
	void initDescriptorSetLayout() override;
	void initPipelineLayout() override;
	void initPipeline() override;
	void initDescriptorPool() override;
	void setupContribution(Contribution& contribution) override;
};

}

#endif
