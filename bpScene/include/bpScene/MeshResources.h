#ifndef BP_MESHRESOURCES_H
#define BP_MESHRESOURCES_H

#include "Mesh.h"
#include <bp/Buffer.h>

namespace bpScene
{

class MeshResources
{
public:
	MeshResources() :
		topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST},
		offset{0},
		elementCount{0} {}
	MeshResources(bp::Device& device, Mesh& mesh, uint32_t offset, uint32_t count) :
		MeshResources{}
	{
		init(device, mesh, offset, count);
	}

	void init(bp::Device& device, const Mesh& mesh)
	{
		init(device, mesh, 0, mesh.getElementCount());
	}
	void init(bp::Device& device, const Mesh& mesh, uint32_t offset, uint32_t count);
	void bind(VkCommandBuffer cmdBuffer);
	VkPrimitiveTopology getTopology() const { return topology; }
	uint32_t getElementCount() const { return elementCount; }

private:
	bp::Buffer vertexBuffer;
	bp::Buffer indexBuffer;
	VkPrimitiveTopology topology;
	uint32_t offset, elementCount;
};

}

#endif
