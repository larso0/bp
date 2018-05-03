#include <bpScene/Model.h>
#include <stdexcept>
#include <tiny_obj_loader.h>
using namespace std;

namespace bpScene
{

static string getDirectoryOfPath(const string& path)
{
	char sep = '/';
#ifdef _WIN32
	sep = '\\';
#endif
	size_t i = path.rfind(sep, path.length());
	if (i != string::npos) {
		return(path.substr(0, i) + sep);
	}
	return("");
}

void Model::loadObj(const std::string& path, const Mesh::LoadFlags& loadFlags)
{
	tinyobj::attrib_t attrib;
	vector <tinyobj::shape_t> shapes;
	vector <tinyobj::material_t> materials;
	string err;

	auto dir = getDirectoryOfPath(path);
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str(), dir.c_str()))
		throw runtime_error(err);

	meshes.resize(shapes.size());
	meshMaterialIndices.resize(shapes.size());
	Model::materials.resize(materials.size());

	for (unsigned i = 0; i < shapes.size(); i++)
	{
		meshes[i].loadShape(attrib, shapes[i], loadFlags);

		const auto& minV = meshes[i].getMinVertex();
		const auto& maxV = meshes[i].getMaxVertex();
		if (minV.x < minVertex.x) minVertex.x = minV.x;
		if (maxV.x > maxVertex.x) maxVertex.x = maxV.x;
		if (minV.y < minVertex.y) minVertex.y = minV.y;
		if (maxV.y > maxVertex.y) maxVertex.y = maxV.y;
		if (minV.z < minVertex.z) minVertex.z = minV.z;
		if (maxV.z > maxVertex.z) maxVertex.z = maxV.z;

		meshMaterialIndices[i] = static_cast<unsigned>(shapes[i].mesh.material_ids[0]);
	}

	for (unsigned i = 0; i < materials.size(); i++)
		Model::materials[i].loadMtl(materials[i], dir);
}

}