#ifndef BP_SCENE_MESH_H
#define BP_SCENE_MESH_H

#include "Vertex.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <initializer_list>
#include <string>

extern "C" {
	enum
	{
		BP_SCENE_MESH_LOAD_POSITION_BIT = 0x01,
		BP_SCENE_MESH_LOAD_NORMAL_BIT = 0x02,
		BP_SCENE_MESH_LOAD_TEXTURE_COORDINATE_BIT = 0x04
	};
};

namespace bpScene
{

using MeshLoadFlags = int;

class Mesh
{
public:
	Mesh() :
		topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST) {}

	void loadObj(const std::string& filename,
		     MeshLoadFlags flags = static_cast<MeshLoadFlags>(0x07));

	uint32_t addVertex(const Vertex& vertex)
	{
		uint32_t i = static_cast<uint32_t>(vertices.size());
		vertices.push_back(vertex);
		return i;
	}

	uint32_t addVertex(const glm::vec3& position, const glm::vec3& normal,
			   const glm::vec2& textureCoordinate)
	{
		return addVertex(Vertex(position, normal, textureCoordinate));
	}

	void addIndices(std::initializer_list<uint32_t> indices)
	{
		for (uint32_t i : indices) this->indices.push_back(i);
	}

	void setTopology(VkPrimitiveTopology topology)
	{
		this->topology = topology;
	}

	VkPrimitiveTopology getTopology() const { return topology; }
	const std::vector<Vertex>& getVertices() const { return vertices; }
	const std::vector<uint32_t>& getIndices() const { return indices; }
	const void* getVertexDataPtr() const { return static_cast<const void*>(vertices.data()); }
	const void* getIndexDataPtr() const { return static_cast<const void*>(indices.data()); }
	size_t getVertexDataSize() const { return vertices.size() * Vertex::STRIDE; }
	size_t getIndexDataSize() const { return indices.size() * sizeof(uint32_t); }
	uint32_t getElementCount() const { return static_cast<uint32_t>(indices.size()); }

private:
	VkPrimitiveTopology topology;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

}

#endif