#ifndef VULKANPERMESH_H
#define VULKANPERMESH_H

#include "VulkanRenderer.h"
#include <vector>

class VulkanPerMesh {
public:
	VulkanPerMesh(VulkanRenderer& renderer, Engine::Mesh* mesh);
	virtual ~VulkanPerMesh();

	void record(VkCommandBuffer cmdBuffer);

//private:
	VulkanRenderer& renderer;

	struct PerBuffer {
		VkBuffer buffer;
		VkDeviceMemory memory;
	};

	std::vector<PerBuffer> buffers;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	void createVertexBuffer(Engine::Mesh* mesh);
	void createPipeline();
};

#endif
