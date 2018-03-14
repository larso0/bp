#include <bpMulti/SortLastCompositor.h>
#include "SortLastCompositingSpv.inc"

namespace bpMulti
{

void SortLastCompositor::resize(uint32_t width, uint32_t height)
{
	//TODO resize pipeline step resources

	Renderer::resize(width, height);
}

void SortLastCompositor::render(bp::Framebuffer& fbo, VkCommandBuffer cmdBuffer)
{
	//TODO perform pipeline steps for producing a frame

	Renderer::render(fbo, cmdBuffer);
}

void SortLastCompositor::setupSubpasses()
{
	subpass.addColorAttachment(getColorAttachmentSlot());
	subpass.setDepthAttachment(getDepthAttachmentSlot());
	addSubpassGraph(subpass);
}

void SortLastCompositor::initResources(uint32_t width, uint32_t height)
{
	initShaders();
	initDescriptorSetLayout();
	initPipelineLayout();
	initPipeline();

	drawable.init(pipeline);
	subpass.addDrawable(drawable);

	//TODO add resource binding delegate to drawable for push descriptors
}

void SortLastCompositor::initShaders()
{
	vertexShader.init(getDevice(), VK_SHADER_STAGE_VERTEX_BIT,
			  static_cast<uint32_t>(fullscreenQuad_vert_spv_len),
			  reinterpret_cast<const uint32_t*>(fullscreenQuad_vert_spv));
	fragmentShader.init(getDevice(), VK_SHADER_STAGE_FRAGMENT_BIT,
			    static_cast<uint32_t>(directTextureDepthTest_frag_spv_len),
			    reinterpret_cast<const uint32_t*>(directTextureDepthTest_frag_spv));
}

void SortLastCompositor::initDescriptorSetLayout()
{
	descriptorSetLayout.addLayoutBinding({0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					      1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	descriptorSetLayout.addLayoutBinding({1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					      1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	descriptorSetLayout.init(getDevice(),
				 VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR);
}

void SortLastCompositor::initPipelineLayout()
{
	pipelineLayout.addDescriptorSetLayout(descriptorSetLayout);
	pipelineLayout.init(getDevice());
}

void SortLastCompositor::initPipeline()
{
	pipeline.addShaderStageInfo(vertexShader.getPipelineShaderStageInfo());
	pipeline.addShaderStageInfo(fragmentShader.getPipelineShaderStageInfo());
	pipeline.setFrontFace(VK_FRONT_FACE_CLOCKWISE);
	pipeline.init(getDevice(), getRenderPass(), pipelineLayout);
}

}