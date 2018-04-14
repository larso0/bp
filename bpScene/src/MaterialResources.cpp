#include <bpScene/MaterialResources.h>
#include <bp/Buffer.h>

using namespace bp;

namespace bpScene
{

void MaterialResources::init(Device& device, const Material& material,
			     DescriptorPool& descriptorPool,
			     bp::DescriptorSetLayout& descriptorSetLayout,
			     uint32_t textureBinding, uint32_t uniformBinding,
			     Buffer& uniformBuffer, VkDeviceSize offset)
{
	descriptorSet.init(device, descriptorPool, descriptorSetLayout);
	if (material.isTextured())
	{
		texture.load(device, VK_IMAGE_USAGE_SAMPLED_BIT, material.getTexturePath());
		texture.transitionShaderReadable(VK_NULL_HANDLE,
						 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		texture.setDescriptorBinding(textureBinding);
		descriptorSet.bind(texture.getDescriptor());
	}

	uniformBufferDescriptor.setType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	uniformBufferDescriptor.setBinding(uniformBinding);
	uniformBufferDescriptor.addDescriptorInfo({uniformBuffer.getHandle(), offset,
						   sizeof(MaterialUniform)});
	descriptorSet.bind(uniformBufferDescriptor);

	descriptorSet.update();

	MaterialUniform& uniform = *static_cast<MaterialUniform*>(uniformBuffer.map() + offset);
	uniform.ambient = {material.getAmbient(), 1.f};
	uniform.diffuse = {material.getDiffuse(), 1.f};
}

}