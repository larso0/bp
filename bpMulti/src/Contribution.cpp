#include <bpMulti/Contribution.h>

using namespace bp;

namespace bpMulti
{

Contribution::~Contribution()
{
	for (Texture* t : textures) delete t;
}

void Contribution::init(Device& device, DescriptorPool& descriptorPool,
			DescriptorSetLayout& descriptorSetLayout, PipelineLayout& pipelineLayout,
			uint32_t width, uint32_t height)
{
	Contribution::device = &device;
	Contribution::width = width;
	Contribution::height = height;
	Contribution::pipelineLayout = &pipelineLayout;
	descriptorSet.init(device, descriptorPool, descriptorSetLayout);
	descriptorSet.update();
}

void Contribution::resize(uint32_t width, uint32_t height)
{
	Contribution::width = width;
	Contribution::height = height;
	for (Texture* t : textures) t->resize(width, height);
	descriptorSet.update();
}

unsigned Contribution::addTexture(VkFormat format)
{
	uint32_t idx = static_cast<uint32_t>(textures.size());
	textures.push_back(new Texture(*device, format, VK_IMAGE_USAGE_SAMPLED_BIT, width, height));
	textures[idx]->setDescriptorBinding(idx);
	descriptorSet.bind(textures[idx]->getDescriptor());
}

void Contribution::bind(VkCommandBuffer cmdBuffer)
{
	VkDescriptorSet set = descriptorSet;
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipelineLayout,
				0, 1, &set, 0, nullptr);
}

}