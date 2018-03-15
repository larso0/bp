#include <bpMulti/SortLastCompositor.h>
#include "SortLastCompositingSpv.inc"
#include <stdexcept>
#include <future>

using namespace bp;
using namespace std;

namespace bpMulti
{

void SortLastCompositor::init(initializer_list<pair<Device*, SortLastRenderer*>> configurations,
			      VkFormat colorFormat, uint32_t width, uint32_t height)
{
	if (configurations.size() < 1)
	{
		throw invalid_argument("At least one configuration must be specified.");
	}
	auto iter = configurations.begin();

	compositingDrawables.resize(configurations.size());
	secondaryDevices.reserve(configurations.size() - 1);
	secondaryRenderSteps.reserve(configurations.size() - 1);
	deviceToHostSteps.reserve(configurations.size() - 1);
	hostCopySteps.reserve(configurations.size() - 1);

	bp::Device* primaryDevice = iter->first;
	primaryRenderStep.init(*primaryDevice, 2, width, height, *iter->second);

	iter++;
	for (; iter != configurations.end(); iter++)
	{
		secondaryDevices.push_back(iter->first);

		secondaryRenderSteps.emplace_back();
		secondaryRenderSteps.back().init(*iter->first, 1, width, height, *iter->second);

		deviceToHostSteps.emplace_back();
		deviceToHostSteps.back().init(true, 2);

		hostCopySteps.emplace_back();
		hostCopySteps.back().init(*primaryDevice, true, width, height, 1);
	}

	hostToDeviceStep.init(*primaryDevice, true, width, height, 1,
			      static_cast<unsigned int>(configurations.size() - 1));

	Compositor::init(*primaryDevice, colorFormat, width, height);
}

void SortLastCompositor::resize(uint32_t width, uint32_t height)
{
	primaryRenderStep.resize(width, height);
	for (auto& step : secondaryRenderSteps) step.resize(width, height);
	for (auto& step : hostCopySteps) step.resize(width, height);
	hostToDeviceStep.resize(width, height);

	Renderer::resize(width, height);
}

void SortLastCompositor::render(Framebuffer& fbo, VkCommandBuffer cmdBuffer)
{
	unsigned nextFrameIndex = (currentFrameIndex + 1) % 2;

	auto primaryRenderFuture = async(launch::async, [this, nextFrameIndex]{
		primaryRenderStep.execute(nextFrameIndex);
	});

	vector<BufferPair> secondaryCopied(secondaryDevices.size());
	vector<future<void>> renderFutures;
	vector<future<void>> hostCopyFutures;
	for (unsigned i = 0; i < secondaryDevices.size(); i++)
	{
		renderFutures.push_back(async(launch::async, [this, nextFrameIndex, i]{
			deviceToHostSteps[i].execute(nextFrameIndex,
						     secondaryRenderSteps[i].execute(0));
		}));

		hostCopyFutures.push_back(async(launch::async, [this, &secondaryCopied, i]{
			auto& p = hostCopySteps[i].
				execute(0, deviceToHostSteps[i].getOutput(currentFrameIndex));
			secondaryCopied[i].first = p.first;
			secondaryCopied[i].second = p.second;
		}));
	}
	for (auto& f : hostCopyFutures) f.wait();
	primaryRenderFuture.wait();

	hostToDeviceStep.execute(0, secondaryCopied, cmdBuffer);

	Renderer::render(fbo, cmdBuffer);

	currentFrameIndex = nextFrameIndex;
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

	for (unsigned i = 0; i < compositingDrawables.size(); i++)
	{
		auto& current = compositingDrawables[i];
		current.init(pipeline);
		subpass.addDrawable(current);

		bpUtil::connect(current.resourceBindingEvent, [this, i](VkCommandBuffer cmdBuffer){
			VkWriteDescriptorSet writes[2];

			if (i == 0)
			{
				auto& fb = primaryRenderStep.getOutput(currentFrameIndex);
				writes[0] = fb.getColorAttachment().getDescriptor().getWriteInfo();
				writes[1] = fb.getDepthAttachment().getDescriptor().getWriteInfo();
			} else
			{
				auto& textures = hostToDeviceStep.getOutput(0)[i - 1];
				writes[0] = textures.first->getDescriptor().getWriteInfo();
				writes[1] = textures.second->getDescriptor().getWriteInfo();
			}

			vkCmdPushDescriptorSetKHR(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
						  pipelineLayout, 0, 2, writes);
		});
	}
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