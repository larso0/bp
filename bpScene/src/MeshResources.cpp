#include <bpScene/MeshResources.h>

using namespace bp;
using namespace std;

namespace bpScene
{

void MeshResources::init(Device& device, const Mesh& mesh, uint32_t offset, uint32_t count)
{
	MeshResources::offset = offset;
	MeshResources::elementCount = count;

	vertexBuffer.init(device, mesh.getVertexDataSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			  VMA_MEMORY_USAGE_GPU_ONLY);
	vertexBuffer.transfer(0, VK_WHOLE_SIZE, mesh.getVertexDataPtr());

	indexBuffer.init(device, mesh.getIndexDataSize(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			 VMA_MEMORY_USAGE_GPU_ONLY);
	indexBuffer.transfer(0, VK_WHOLE_SIZE, mesh.getIndexDataPtr());
}

void MeshResources::bind(VkCommandBuffer cmdBuffer)
{
	VkDeviceSize vertexBufferOffset = 0;
	VkDeviceSize indexBufferOffset = offset * sizeof(uint32_t);
	VkBuffer vertexBufferHandle = vertexBuffer;
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &vertexBufferHandle, &vertexBufferOffset);
	vkCmdBindIndexBuffer(cmdBuffer, indexBuffer, indexBufferOffset, VK_INDEX_TYPE_UINT32);
}

}
