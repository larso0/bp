#include <bpMulti/Contribution.h>

using namespace bp;

namespace bpMulti
{

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

void Contribution::resize(uint32_t width, uint32_t height, bool resizeTextures)
{
	Contribution::width = width;
	Contribution::height = height;
	if (resizeTextures) for (auto& t : textures) t->resize(width, height);
	descriptorSet.update();
}

unsigned Contribution::createTexture(VkFormat format, bool addToDescriptorSet)
{
	uint32_t idx = static_cast<uint32_t>(textures.size());
	textures.emplace_back(
		new Texture(*device, format, VK_IMAGE_USAGE_SAMPLED_BIT, width, height));
	if (addToDescriptorSet)
	{
		textures[idx]->setDescriptorBinding(idx);
		descriptorSet.bind(textures[idx]->getDescriptor());
	}
	return idx;
}

unsigned Contribution::addTexture(bp::Texture& texture, bool addToDescriptorSet)
{
	uint32_t idx = static_cast<uint32_t>(textures.size());
	textures.emplace_back(&texture, [](Texture*){});
	if (addToDescriptorSet)
	{
		textures[idx]->setDescriptorBinding(idx);
		descriptorSet.bind(textures[idx]->getDescriptor());
	}
	return idx;
}

void Contribution::flushStagingBuffer(unsigned index, VkCommandBuffer cmdBuffer)
{
	getTexture(index).getImage().flushStagingBuffer(cmdBuffer);
}

void Contribution::transitionTextureShaderReadable(unsigned index, VkCommandBuffer cmdBuffer)
{
	getTexture(index).transitionShaderReadable(cmdBuffer,
						   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

void Contribution::bind(VkCommandBuffer cmdBuffer)
{
	VkDescriptorSet set = descriptorSet;
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipelineLayout,
				0, 1, &set, 0, nullptr);
}

}