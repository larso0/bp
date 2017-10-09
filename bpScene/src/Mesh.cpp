#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <bpScene/Mesh.h>
#include <unordered_map>
#include <stdexcept>
#include <glm/gtx/hash.hpp>

namespace std
{
template<>
struct hash<bpScene::Vertex>
{
	size_t operator()(bpScene::Vertex const& vertex) const
	{
		return ((hash<glm::vec3>()(vertex.position) ^
			 (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
		       (hash<glm::vec2>()(vertex.textureCoordinate) << 1);
	}
};
}

using namespace std;
using glm::vec3;
using glm::vec2;

namespace bpScene
{

const uint32_t Vertex::STRIDE = sizeof(Vertex);
const uint32_t Vertex::POSITION_OFFSET = 0;
const uint32_t Vertex::NORMAL_OFFSET = sizeof(glm::vec3);
const uint32_t Vertex::TEXTURE_COORDINATE_OFFSET = 2 * sizeof(glm::vec3);

void Mesh::loadObj(const string& filename, MeshLoadFlags flags)
{
	tinyobj::attrib_t attrib;
	vector <tinyobj::shape_t> shapes;
	vector <tinyobj::material_t> materials;
	string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str()))
		throw runtime_error(err);

	unordered_map<Vertex, uint32_t> uniqueVertices;

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex;
			if (flags & BP_SCENE_MESH_LOAD_POSITION_BIT)
				vertex.position = vec3(attrib.vertices[3 * index.vertex_index],
						       attrib.vertices[3 * index.vertex_index + 1],
						       attrib.vertices[3 * index.vertex_index + 2]);
			if (flags & BP_SCENE_MESH_LOAD_NORMAL_BIT)
				vertex.normal = vec3(attrib.normals[3 * index.normal_index],
						     attrib.normals[3 * index.normal_index + 1],
						     attrib.normals[3 * index.normal_index + 2]);
			if (flags & BP_SCENE_MESH_LOAD_TEXTURE_COORDINATE_BIT)
				vertex.textureCoordinate =
					vec2(attrib.texcoords[2 * index.texcoord_index],
					     1.f - attrib.texcoords[2 * index.texcoord_index + 1]);

			if (uniqueVertices.count(vertex) == 0)
				uniqueVertices[vertex] = addVertex(vertex);

			indices.push_back(uniqueVertices[vertex]);
		}
	}
}

}