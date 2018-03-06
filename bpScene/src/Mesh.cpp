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
		return (hash<glm::vec3>()(vertex.position));
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

void Mesh::loadObj(const string& filename, const LoadFlags& flags)
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
			if (flags & LoadFlag::POSITION)
				vertex.position = vec3(attrib.vertices[3 * index.vertex_index],
						       attrib.vertices[3 * index.vertex_index + 1],
						       attrib.vertices[3 * index.vertex_index + 2]);

			if (uniqueVertices.count(vertex) == 0)
				uniqueVertices[vertex] = addVertex(vertex);

			indices.push_back(uniqueVertices[vertex]);
		}
	}
}

}