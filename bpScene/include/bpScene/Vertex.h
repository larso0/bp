#ifndef BP_SCENE_VERTEX_H
#define BP_SCENE_VERTEX_H

#include "Math.h"

namespace bpScene
{

class Vertex
{
public:
	Vertex() {}
	Vertex(const glm::vec3& p, const glm::vec3& n, const glm::vec2& uv) :
		position(p),
		normal(n),
		textureCoordinate(uv) {}

	bool operator==(const Vertex& other) const
	{
		return position == other.position && normal == other.normal &&
		       textureCoordinate == other.textureCoordinate;
	}

	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 textureCoordinate;

	static const uint32_t STRIDE;
	static const uint32_t POSITION_OFFSET;
	static const uint32_t NORMAL_OFFSET;
	static const uint32_t TEXTURE_COORDINATE_OFFSET;
};

}

#endif