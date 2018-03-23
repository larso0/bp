#include <bpMulti/SortFirstCompositor.h>
#include "SortFirstCompositingSpv.inc"

using namespace bp;
using namespace std; 

namespace bpMulti
{

void SortFirstCompositor::init(initializer_list<pair<Device*, SortFirstRenderer*>> configurations,
			       VkFormat colorFormat, uint32_t width, uint32_t height)
{
	deviceCount = static_cast<unsigned int>(configurations.size());
	contributionRegions.resize(deviceCount);
	unsigned regionHeight = height / deviceCount;
	for (unsigned i = 0; i < deviceCount; i++)
	{
		contributionRegions[i].offset = {0, static_cast<int32_t>(regionHeight * i)};
		contributionRegions[i].extent = {width, regionHeight};
	}
	contributionRegions.back().extent.height += height % deviceCount;

	auto iter = configurations.begin();
	bp::Device& primaryDevice = *iter->first;
	primaryRenderer = iter->second;
	unsigned i = 0;
	auto& r = contributionRegions[i];
	iter->second->setContribution(
		{static_cast<float>(r.offset.x) / static_cast<float>(width),
		 static_cast<float>(r.offset.y) / static_cast<float>(height)},
		{static_cast<float>(r.extent.width) / static_cast<float>(width),
		 static_cast<float>(r.extent.height) / static_cast<float>(height)});
	iter++;
	while (iter != configurations.end())
	{
		secondaryDevices.push_back(iter->first);
		secondaryRenderers.push_back(iter->second);
		auto& r = contributionRegions[i];
		iter->second->setContribution(
			{static_cast<float>(r.offset.x) / static_cast<float>(width),
			 static_cast<float>(r.offset.y) / static_cast<float>(height)},
			{static_cast<float>(r.extent.width) / static_cast<float>(width),
			 static_cast<float>(r.extent.height) / static_cast<float>(height)});
		iter++;
		i++;
	}

	Compositor::init(primaryDevice, colorFormat, width, height);

	for (unsigned i = 0; i < deviceCount; i++)
	{
		bpUtil::connect(drawables[i].resourceBindingEvent,
				[this, i](VkCommandBuffer cmdBuffer){
					auto& r = contributionRegions[i];
					PushConstants push{
						{static_cast<float>(r.offset.x)
						 / static_cast<float>(getWidth()),
						 static_cast<float>(r.offset.y)
						 / static_cast<float>(getHeight())},
						{static_cast<float>(r.extent.width)
						 / static_cast<float>(getWidth()),
						 static_cast<float>(r.extent.height)
						 / static_cast<float>(getHeight())}
					};
					push.offset *= 2.f;
					push.offset -= 1.f;
					push.extent *= 2.f;
					vkCmdPushConstants(cmdBuffer, pipelineLayout,
							   VK_SHADER_STAGE_VERTEX_BIT, 0,
							   sizeof(push), &push);
				});
	}
}

void SortFirstCompositor::resize(uint32_t width, uint32_t height)
{
	unsigned regionHeight = height / deviceCount;
	for (unsigned i = 0; i < deviceCount; i++)
	{
		contributionRegions[i].offset = {0, static_cast<int32_t>(regionHeight * i)};
		contributionRegions[i].extent = {width, regionHeight};
	}
	contributionRegions.back().extent.height += height % deviceCount;

	auto& region = contributionRegions[0];
	auto renderer = static_cast<SortFirstRenderer*>(primaryRenderer);
	renderer->setContribution(
		{static_cast<float>(region.offset.x) / static_cast<float>(width),
		 static_cast<float>(region.offset.y) / static_cast<float>(height)},
		{static_cast<float>(region.extent.width) / static_cast<float>(width),
		 static_cast<float>(region.extent.height) / static_cast<float>(height)});
	primaryRenderDeviceSteps.resize(region.extent.width, region.extent.height);
	for (auto& c : primaryContributions)
		c.resize(region.extent.width, region.extent.height, false);

	for (unsigned i = 1; i < deviceCount; i++)
	{
		renderer = static_cast<SortFirstRenderer*>(secondaryRenderers[i - 1]);
		auto& region = contributionRegions[i];
		renderer->setContribution(
			{static_cast<float>(region.offset.x) / static_cast<float>(width),
			 static_cast<float>(region.offset.y) / static_cast<float>(height)},
			{static_cast<float>(region.extent.width) / static_cast<float>(width),
			 static_cast<float>(region.extent.height) / static_cast<float>(height)});
		secondaryRenderDeviceSteps[i - 1].resize(region.extent.width, region.extent.height);
		secondaryContributions[i - 1].resize(region.extent.width, region.extent.height);
	}

	Renderer::resize(width, height);
}

VkExtent2D SortFirstCompositor::getContributionSize(unsigned deviceIndex)
{
	return contributionRegions[deviceIndex].extent;
}

void SortFirstCompositor::initShaders()
{
	vertexShader.init(getDevice(), VK_SHADER_STAGE_VERTEX_BIT, compositeQuad_vert_spv_len,
			  reinterpret_cast<const uint32_t*>(compositeQuad_vert_spv));
	fragmentShader.init(getDevice(), VK_SHADER_STAGE_FRAGMENT_BIT,
			    directTexture_frag_spv_len,
			    reinterpret_cast<const uint32_t*>(directTexture_frag_spv));
}

void SortFirstCompositor::initDescriptorSetLayout()
{
	descriptorSetLayout.addLayoutBinding({0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					      1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
	descriptorSetLayout.init(getDevice());
}

void SortFirstCompositor::initPipelineLayout()
{
	pipelineLayout.addDescriptorSetLayout(descriptorSetLayout);
	pipelineLayout.addPushConstantRange({VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants)});
	pipelineLayout.init(getDevice());
}

void SortFirstCompositor::initPipeline()
{
	pipeline.addShaderStageInfo(vertexShader.getPipelineShaderStageInfo());
	pipeline.addShaderStageInfo(fragmentShader.getPipelineShaderStageInfo());
	pipeline.setFrontFace(VK_FRONT_FACE_CLOCKWISE);
	pipeline.setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
	pipeline.init(getDevice(), getRenderPass(), pipelineLayout);
}

void SortFirstCompositor::initDescriptorPool()
{
	descriptorPool.init(getDevice(),
			    {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, deviceCount + 1}},
			    deviceCount + 1);
}

void SortFirstCompositor::setupContribution(Contribution& contribution)
{
	contribution.createTexture(VK_FORMAT_R8G8B8A8_UNORM, true);
}

}