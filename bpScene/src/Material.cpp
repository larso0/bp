#include <bpScene/Material.h>
#include <tiny_obj_loader.h>

namespace bpScene
{

void Material::loadMtl(const tinyobj::material_t& mtl)
{
	texturePath = mtl.diffuse_texname;
	ambient = {mtl.ambient[0], mtl.ambient[1], mtl.ambient[2]};
	diffuse = {mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2]};
}

}