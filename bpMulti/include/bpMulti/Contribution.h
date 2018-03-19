#ifndef BP_CONTRIBUTION_H
#define BP_CONTRIBUTION_H

#include <bp/Texture.h>
#include <bp/DescriptorSet.h>
#include <vector>
#include <bp/PipelineLayout.h>
#include <bp/OffscreenFramebuffer.h>

namespace bpMulti
{

class Contribution
{
public:
	Contribution() : device{nullptr}, width{0}, height{0} {}
	~Contribution();

	void init(bp::Device& device, bp::DescriptorPool& descriptorPool,
		  bp::DescriptorSetLayout& descriptorSetLayout, bp::PipelineLayout& pipelineLayout,
		  uint32_t width, uint32_t height);
	void resize(uint32_t width, uint32_t height);
	unsigned addTexture(VkFormat format);
	void flushStagingBuffers(VkCommandBuffer cmdBuffer);
	void update() { descriptorSet.update(); }
	void bind(VkCommandBuffer cmdBuffer);

	unsigned getTextureCount() const { return static_cast<unsigned>(textures.size()); }
	bp::Texture& getTexture(unsigned index) { return *textures[index]; }

private:
	bp::Device* device;
	uint32_t width, height;
	std::vector<bp::Texture*> textures;
	bp::PipelineLayout* pipelineLayout;
	bp::DescriptorSet descriptorSet;
};

}

#endif