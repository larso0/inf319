#include "VulkanPerMesh.h"
#include <Engine/IndexedMesh.h>
#include <stdexcept>

using namespace std;
using namespace Engine;

VulkanPerMesh::VulkanPerMesh(const VulkanDevice& device, const Engine::Mesh* mesh) :
	device(device),
	topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST),
	indexed(false),
	elementCount(0)
{
	createBuffers(mesh);

	switch (mesh->getTopology()) {
	case Mesh::Topology::Points:
		topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		break;
	case Mesh::Topology::Lines:
		topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		break;
	case Mesh::Topology::LineStrip:
		topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		break;
	case Mesh::Topology::Triangles:
		topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		break;
	case Mesh::Topology::TriangleStrip:
		topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		break;
	case Mesh::Topology::TriangleFan:
		topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		break;
	}
}

VulkanPerMesh::~VulkanPerMesh() {
	for (VulkanBuffer* b : buffers) {
		delete b;
	}
}

void VulkanPerMesh::createBuffers(const Mesh* mesh) {
	VulkanBuffer* vertexBuffer = new VulkanBuffer(device,
		mesh->getVertexDataSize(),
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			| VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	vertexBuffer->transfer(0, VK_WHOLE_SIZE, (void*)mesh->getVertexData());
	buffers.push_back(vertexBuffer);

	const IndexedMesh* indexedMesh = dynamic_cast<const IndexedMesh*>(mesh);
	if (indexedMesh) {
		indexed = true;
		elementCount = (uint32_t)indexedMesh->getElementCount();

		VulkanBuffer* indexBuffer = new VulkanBuffer(device,
			indexedMesh->getIndexDataSize(),
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
				| VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		indexBuffer->transfer(0, VK_WHOLE_SIZE,
			(void*)indexedMesh->getIndexData());
		buffers.push_back(indexBuffer);
	} else {
		elementCount = (uint32_t)mesh->getElementCount();
	}
}

void VulkanPerMesh::record(VkCommandBuffer cmdBuffer) {
	VkDeviceSize offset = 0;
	VkBuffer bufferHandle = buffers[0]->getHandle();
	vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &bufferHandle, &offset);

	if (indexed) {
		vkCmdBindIndexBuffer(cmdBuffer, buffers[1]->getHandle(), 0,
			VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmdBuffer, elementCount, 1, 0, 0, 0);
	} else {
		vkCmdDraw(cmdBuffer, elementCount, 1, 0, 0);
	}
}
