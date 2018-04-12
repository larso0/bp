#include <bpScene/Model.h>
#include <stdexcept>
#include <tiny_obj_loader.h>
using namespace std;

namespace bpScene
{

void Model::loadObj(const std::string& path)
{
	tinyobj::attrib_t attrib;
	vector <tinyobj::shape_t> shapes;
	vector <tinyobj::material_t> materials;
	string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str()))
		throw runtime_error(err);

	meshes.resize(shapes.size());
	Model::materials.resize(materials.size());

	for (unsigned i = 0; i < shapes.size(); i++)
	{
		meshes[i].loadShape(attrib, shapes[i]);
		meshMaterialIndices[i] = static_cast<unsigned>(shapes[i].mesh.material_ids[0]);
	}

	for (unsigned i = 0; i < materials.size(); i++)
		Model::materials[i].loadMtl(materials[i]);
}

}