#include <bpScene/MeshResources.h>

using namespace bp;
using namespace std;

namespace bpScene
{

void MeshResources::init(Device& device, const Mesh& mesh, uint32_t offset, uint32_t count)
{
	MeshResources::offset = offset;
	MeshResources::elementCount = count;
	indexBufferOffset = offset * sizeof(uint32_t);

	unsigned bufferCount = 2;
	if (mesh.haveNormals()) bufferCount++;
	if (mesh.haveTexCoords()) bufferCount++;

	buffers.resize(bufferCount);

	buffers[0].init(device, mesh.getIndexDataSize(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY);
	buffers[0].transfer(0, VK_WHOLE_SIZE, mesh.getIndexDataPtr());
	buffers[0].freeStagingBuffer();

	buffers[1].init(device, mesh.getPositionDataSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			  VMA_MEMORY_USAGE_GPU_ONLY);
	buffers[1].transfer(0, VK_WHOLE_SIZE, mesh.getPositionDataPtr());
	buffers[1].freeStagingBuffer();
	vertexBufferOffsets.push_back(0);
	vertexBufferHandles.push_back(buffers[1].getHandle());

	if (mesh.haveNormals())
	{
		buffers[2].init(device, mesh.getNormalDataSize(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VMA_MEMORY_USAGE_GPU_ONLY);
		buffers[2].transfer(0, VK_WHOLE_SIZE, mesh.getNormalDataPtr());
		buffers[2].freeStagingBuffer();
		vertexBufferOffsets.push_back(0);
		vertexBufferHandles.push_back(buffers[2].getHandle());
	}
	if (mesh.haveTexCoords())
	{
		buffers[3].init(device, mesh.getTexCoordDataSize(),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		buffers[3].transfer(0, VK_WHOLE_SIZE, mesh.getTexCoordDataPtr());
		buffers[3].freeStagingBuffer();
		vertexBufferOffsets.push_back(0);
		vertexBufferHandles.push_back(buffers[3].getHandle());
	}
}

void MeshResources::bind(VkCommandBuffer cmdBuffer)
{
	vkCmdBindVertexBuffers(cmdBuffer, 0, static_cast<uint32_t>(vertexBufferHandles.size()),
			       vertexBufferHandles.data(), vertexBufferOffsets.data());
	vkCmdBindIndexBuffer(cmdBuffer, buffers[0], indexBufferOffset, VK_INDEX_TYPE_UINT32);
}

}
