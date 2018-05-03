#ifndef BP_SCENE_MESH_H
#define BP_SCENE_MESH_H

#include "Math.h"
#include <bpUtil/FlagSet.h>
#include <vulkan/vulkan.h>
#include <tiny_obj_loader.h>
#include <vector>
#include <initializer_list>
#include <string>

namespace bpScene
{

class Mesh
{
public:
	enum LoadFlag
	{
		NORMAL,
		TEXTURE_COORDINATE,
		BP_FLAGSET_LAST
	};
	using LoadFlags = bpUtil::FlagSet<LoadFlag>;

	Mesh(VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) :
		topology{topology},
		maxVertex{FLT_MIN, FLT_MIN, FLT_MIN}, minVertex{FLT_MAX, FLT_MAX, FLT_MAX} {}

	void loadObj(const std::string& filename, const LoadFlags& flags = LoadFlags() << NORMAL);
	void loadShape(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape,
		       const LoadFlags& flags = LoadFlags() << NORMAL << TEXTURE_COORDINATE);

	void addIndices(std::initializer_list<uint32_t> indices)
	{
		for (uint32_t i : indices) this->indices.push_back(i);
	}

	void setTopology(VkPrimitiveTopology topology)
	{
		this->topology = topology;
	}

	VkPrimitiveTopology getTopology() const { return topology; }
	const std::vector<glm::vec3>& getPositions() const { return positions; }
	const std::vector<glm::vec3>& getNormals() const { return normals; }
	const std::vector<glm::vec2>& getTexCoords() const { return texCoords; }
	const std::vector<uint32_t>& getIndices() const { return indices; }
	bool haveNormals() const { return !normals.empty(); }
	bool haveTexCoords() const { return !texCoords.empty(); }
	const void* getPositionDataPtr() const { return static_cast<const void*>(positions.data()); }
	const void* getNormalDataPtr() const { return static_cast<const void*>(normals.data()); }
	const void* getTexCoordDataPtr() const { return static_cast<const void*>(texCoords.data()); }
	const void* getIndexDataPtr() const { return static_cast<const void*>(indices.data()); }
	size_t getPositionDataSize() const { return positions.size() * sizeof(glm::vec3); }
	size_t getNormalDataSize() const { return normals.size() * sizeof(glm::vec3); }
	size_t getTexCoordDataSize() const { return texCoords.size() * sizeof(glm::vec2); }
	size_t getIndexDataSize() const { return indices.size() * sizeof(uint32_t); }
	uint32_t getElementCount() const { return static_cast<uint32_t>(indices.size()); }
	const glm::vec3& getMaxVertex() const { return maxVertex; }
	const glm::vec3& getMinVertex() const { return minVertex; }

private:
	VkPrimitiveTopology topology;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<uint32_t> indices;
	glm::vec3 maxVertex, minVertex;
};

}

#endif