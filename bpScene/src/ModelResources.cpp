#include <bpScene/ModelResources.h>

namespace bpScene
{

void ModelResources::init(bp::Device& device, bp::DescriptorSetLayout& descriptorSetLayout,
			  uint32_t textureBinding, uint32_t uniformBinding, const Model& model)
{
	meshMaterialIndices = model.meshMaterialIndices;

	meshes.resize(model.getMeshCount());

	for (unsigned i = 0; i < model.getMeshCount(); i++)
	{
		meshes[i].init(device, model.getMesh(i));
	}

	auto& limits = device.getProperties().limits;

	VkDeviceSize alignment = limits.minUniformBufferOffsetAlignment;
	uniformStride = alignment;
	while (uniformStride < sizeof(MaterialResources::MaterialUniform))
		uniformStride += alignment;
	VkDeviceSize uniformBufferSize = model.getMaterialCount() * uniformStride;
	uniformBuffer.init(device, uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			   VMA_MEMORY_USAGE_GPU_ONLY);

	descriptorPool.init(device,
			    {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, model.getMaterialCount()},
			     {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, model.getMaterialCount()}},
			    model.getMaterialCount());

	materials.resize(model.getMaterialCount());
	for (unsigned i = 0; i < model.getMaterialCount(); i++)
	{
		bpUtil::connect(materials[i].loadMessageEvent, loadMessageEvent);
		materials[i].init(device, model.getMaterial(i), descriptorPool, descriptorSetLayout,
				  textureBinding, uniformBinding, uniformBuffer, i * uniformStride);
	}
	uniformBuffer.flushStagingBuffer();

}

}