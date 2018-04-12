#ifndef BP_SCENE_MODEL_RESOURCES_H
#define BP_SCENE_MODEL_RESOURCES_H

#include "ResourceList.h"
#include "Model.h"
#include "MeshResources.h"
#include "MaterialResources.h"
#include <bp/Device.h>
#include <bp/Buffer.h>
#include <bp/DescriptorPool.h>
#include <bp/DescriptorSetLayout.h>

namespace bpScene
{

class ModelResources
{
public:
	void init(bp::Device& device, bp::DescriptorSetLayout& descriptorSetLayout,
		  uint32_t textureBinding, uint32_t uniformBinding, const Model& model);

	unsigned getMeshCount() const { return static_cast<unsigned>(meshes.size()); }
	unsigned getMaterialCount() const { return static_cast<unsigned>(materials.size()); }
	MeshResources& getMesh(unsigned index) { return meshes[index];}
	MaterialResources& getMaterial(unsigned index) { return materials[index]; }
	unsigned getMaterialIndexForMesh(unsigned meshIndex) const
	{
		return meshMaterialIndices[meshIndex];
	}
	MaterialResources& getMaterialForMesh(unsigned meshIndex)
	{
		return materials[meshMaterialIndices[meshIndex]];
	}

private:
	std::vector<MeshResources> meshes;
	std::vector<MaterialResources> materials;
	std::vector<unsigned> meshMaterialIndices;

	VkDeviceSize uniformStride;
	bp::Buffer uniformBuffer;
	bp::DescriptorPool descriptorPool;
};

}

#endif