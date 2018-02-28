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
		elementCount{0} {}
	MeshResources(bp::Device& device, Mesh& mesh) :
		MeshResources{}
	{
		init(device, mesh);
	}

	void init(bp::Device& device, Mesh& mesh);
	void bind(VkCommandBuffer cmdBuffer, uint32_t offset = 0);
	VkPrimitiveTopology getTopology() const { return topology; }
	uint32_t getElementCount() const { return elementCount; }

private:
	bp::Buffer vertexBuffer;
	bp::Buffer indexBuffer;
	VkPrimitiveTopology topology;
	uint32_t elementCount;
};

}

#endif