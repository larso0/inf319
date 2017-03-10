#ifndef VULKANPERMESH_H
#define VULKANPERMESH_H

#include <vector>
#include <Engine/Mesh.h>
#include <vulkan/vulkan.h>
#include "VulkanShaderProgram.h"

class VulkanRenderer;

class VulkanPerMesh {
public:
	VulkanPerMesh();
	virtual ~VulkanPerMesh();

	void init(const VulkanShaderProgram& shaderProgram,
		const Engine::Mesh* mesh,
		const VkPhysicalDeviceMemoryProperties& memoryProperties,
		VkViewport* viewport, VkRect2D* scissor,
		VkRenderPass renderPass);
	void record(VkCommandBuffer cmdBuffer, VkViewport* viewport,
		VkRect2D* scissor);

private:
	VkDevice device;

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

	void createBuffers(const Engine::Mesh* mesh,
		const VkPhysicalDeviceMemoryProperties& memoryProperties);
	void createPipeline(
		const std::vector<VkPipelineShaderStageCreateInfo>& stages,
		VkViewport* viewport, VkRect2D* scissor,
		VkRenderPass renderPass);
};

#endif
