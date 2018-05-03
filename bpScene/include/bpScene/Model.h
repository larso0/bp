#ifndef BP_SCENE_MODEL_H
#define BP_SCENE_MODEL_H

#include "Mesh.h"
#include "Material.h"

namespace bpScene
{

class ModelResources;

class Model
{
	friend class ModelResources;
public:
	void loadObj(const std::string& path, const Mesh::LoadFlags& loadFlags);

	unsigned getMeshCount() const { return static_cast<unsigned>(meshes.size()); }
	unsigned getMaterialCount() const { return static_cast<unsigned>(materials.size()); }
	const Mesh& getMesh(unsigned index) const { return meshes[index];}
	const Material& getMaterial(unsigned index) const { return materials[index]; }
	unsigned getMaterialIndexForMesh(unsigned meshIndex) const
	{
		return meshMaterialIndices[meshIndex];
	}
	const Material& getMaterialForMesh(unsigned meshIndex) const
	{
		return materials[meshMaterialIndices[meshIndex]];
	}

	const glm::vec3& getMaxVertex() const { return maxVertex; }
	const glm::vec3& getMinVertex() const { return minVertex; }

private:
	std::vector<Mesh> meshes;
	std::vector<Material> materials;
	std::vector<unsigned> meshMaterialIndices;
	glm::vec3 maxVertex, minVertex;
};

}

#endif