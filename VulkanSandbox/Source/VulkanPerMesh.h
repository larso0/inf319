#ifndef VULKANPERMESH_H
#define VULKANPERMESH_H

#include <vector>
#include <Engine/Mesh.h>
#include <vulkan/vulkan.h>

#include "VulkanBuffer.h"
#include "VulkanDevice.h"

class VulkanPerMesh {
public:
	VulkanPerMesh(const VulkanDevice& device, const Engine::Mesh* mesh);
	virtual ~VulkanPerMesh();

	void record(VkCommandBuffer cmdBuffer);

private:
	const VulkanDevice& device;

	std::vector<VulkanBuffer*> buffers;
	VkPrimitiveTopology topology;
	bool indexed;
	uint32_t elementCount;

	void createBuffers(const Engine::Mesh* mesh);
};

#endif
