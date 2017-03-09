#ifndef VULKANPERMESH_H
#define VULKANPERMESH_H

#include <vector>
#include <Engine/Mesh.h>
#include <vulkan/vulkan.h>

class VulkanRenderer;

class VulkanPerMesh {
public:
	VulkanPerMesh(VulkanRenderer& renderer, const Engine::Mesh* mesh);
	virtual ~VulkanPerMesh();

	void record(VkCommandBuffer cmdBuffer);

//private:
	VulkanRenderer& renderer;

	struct PerBuffer {
		VkBuffer buffer;
		VkDeviceMemory memory;
	};

	std::vector<PerBuffer> buffers;
	VkPrimitiveTopology topology;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	bool indexed;
	uint32_t indexCount;

	void createBuffers(const Engine::Mesh* mesh);
	void createPipeline();
};

#endif
