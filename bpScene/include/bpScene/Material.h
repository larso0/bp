#ifndef BP_SCENE_MATERIAL_H
#define BP_SCENE_MATERIAL_H

#include "Math.h"
#include <string>
#include <tiny_obj_loader.h>

namespace bpScene
{

class Material
{
public:
	Material() {}
	Material(const std::string& texturePath, const glm::vec3& ambient,
		 const glm::vec3& diffuse) :
		texturePath{texturePath},
		ambient{ambient}, diffuse{diffuse} {}

	void loadMtl(const tinyobj::material_t& mtl, const std::string& dir);

	void setTexturePath(const std::string& path) { texturePath = path; }
	void setAmbient(const glm::vec3& color) { ambient = color; }
	void setDiffuse(const glm::vec3& color) { diffuse = color; }

	bool isTextured() const { return !texturePath.empty(); }
	const std::string& getTexturePath() const { return texturePath; }
	const glm::vec3 getAmbient() const { return ambient; }
	const glm::vec3 getDiffuse() const { return diffuse; }

private:
	std::string texturePath;
	glm::vec3 ambient, diffuse;
};

}

#endif