#ifndef BP_COMPOSITERENDERER_H
#define BP_COMPOSITERENDERER_H

#include "Renderer.h"
#include "Buffer.h"
#include "Shader.h"
#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"
#include "DescriptorSet.h"
#include "ImageDescriptor.h"
#include <vector>

namespace bp
{

class CompositeRenderer : public Renderer
{
public:
	CompositeRenderer(RenderPass& renderPass, const std::vector<VkRect2D>& areas);
	~CompositeRenderer();

	void render(VkSemaphore waitSem = VK_NULL_HANDLE) override;
	void draw(VkCommandBuffer) override;

	Image& getSourceImage(uint32_t index)
	{
		return sources[index].image;
	}

private:
	struct PushConstant
	{
		float x, y, w, h;
		uint32_t samplerIndex;
	};

	class CompositeSource
	{
	public:
		CompositeSource(bp::Device& device, const VkRect2D& area, uint32_t samplerIndex);
		~CompositeSource();
		bp::Device& device;
		bp::Image image;
		VkImageView imageView;
		VkSampler sampler;
		PushConstant pushConstant;
	};

	bp::Device& device;
	bp::Shader* vertexShader;
	bp::Shader* fragmentShader;
	bp::DescriptorPool* descriptorPool;
	bp::DescriptorSetLayout* descriptorSetLayout;
	std::vector<bp::DescriptorSet*> descriptorSets;
	std::vector<bp::ImageDescriptor*> imageDescriptors;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	std::vector<CompositeSource> sources;

	void createShaders();
	void createDescriptorPool();
	void createDescriptorSetLayout();
	void createDescriptorSets();
	void setupDescriptors();
	void createPipelineLayout();
	void createPipeline();

};

}

#endif