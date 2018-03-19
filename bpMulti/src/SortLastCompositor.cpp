#include <bpMulti/SortLastCompositor.h>
#include <bp/Util.h>
#include <future>
#include "SortLastCompositingSpv.inc"

using namespace std;

namespace bpMulti
{

void SortLastCompositor::init(initializer_list<pair<bp::Device*, SortLastRenderer*>> configurations,
			      VkFormat colorFormat, uint32_t width, uint32_t height)
{
	deviceCount = static_cast<unsigned int>(configurations.size());
	auto iter = configurations.begin();
	bp::Device& primaryDevice = *iter->first;
	primaryRenderer = iter->second;
	iter++;
	for (; iter != configurations.end(); iter++)
	{
		secondaryDevices.push_back(iter->first);
		secondaryRenderers.push_back(iter->second);
	}
	Compositor::init(primaryDevice, colorFormat, width, height);
}

VkExtent2D SortLastCompositor::getContributionSize(unsigned deviceIndex)
{
	return {getWidth(), getHeight()};
}

void SortLastCompositor::hostCopyStep()
{
	vector<future<void>> futures;
	for (unsigned i = 0; i < deviceCount - 1; i++)
	{
		auto& fb = secondaryRenderDeviceSteps[i].getFramebuffer(currentFrameIndex);
		auto& contribution = secondaryContributions[i + 1];
		futures.push_back(async(launch::async, [&fb, &contribution]{
			auto colorCopyFuture = async(launch::async, [&]{
				size_t colorSize = fb.getWidth() * fb.getHeight() * 4;
				bp::parallelCopy(contribution.getTexture(0).getImage().map(),
						 fb.getColorAttachment().getImage().map(),
						 colorSize);
			});
			size_t depthSize = fb.getWidth() * fb.getHeight() * 2;
			bp::parallelCopy(contribution.getTexture(1).getImage().map(),
					 fb.getDepthAttachment().getImage().map(),
					 depthSize);
			colorCopyFuture.wait();
		}));
	}
}

void SortLastCompositor::initShaders()
{
	vertexShader.init(getDevice(), VK_SHADER_STAGE_VERTEX_BIT, fullscreenQuad_vert_spv_len,
			  reinterpret_cast<const uint32_t*>(fullscreenQuad_vert_spv));
	fragmentShader.init(getDevice(), VK_SHADER_STAGE_FRAGMENT_BIT,
			    directTextureDepthTest_frag_spv_len,
			    reinterpret_cast<const uint32_t*>(directTextureDepthTest_frag_spv));
}

void SortLastCompositor::initDescriptorSetLayout()
{
	descriptorSetLayout.addLayoutBinding({0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					      1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	descriptorSetLayout.addLayoutBinding({1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					      1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	descriptorSetLayout.init(getDevice());
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

void SortLastCompositor::initDescriptorPool()
{
	descriptorPool.init(getDevice(),
			    {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, deviceCount * 2 + 2}},
			    deviceCount + 1);
}

void SortLastCompositor::setupContribution(Contribution& contribution)
{
	contribution.addTexture(VK_FORMAT_R8G8B8A8_UNORM);
	contribution.addTexture(VK_FORMAT_D16_UNORM);
}

}