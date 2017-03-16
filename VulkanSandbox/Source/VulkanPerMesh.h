#ifndef VULKANPERMESH_H
#define VULKANPERMESH_H

#include <vector>
#include <Engine/Mesh.h>
#include <vulkan/vulkan.h>
#include "VulkanShaderProgram.h"
#include "VulkanBuffers.h"

class VulkanPerMesh {
public:
	VulkanPerMesh();
	virtual ~VulkanPerMesh();

	void init(const VulkanShaderProgram& shaderProgram,
		const Engine::Mesh* mesh,
		const VkPhysicalDeviceMemoryProperties& memoryProperties,
		VkViewport* viewport, VkRect2D* scissor,
		VkRenderPass renderPass, VkPipelineLayout pipelineLayout);
	void record(VkCommandBuffer cmdBuffer);

	VkPipeline getPipeline() const {
		return pipeline;
	}

private:
	VkDevice device;

	std::vector<VulkanBuffer> buffers;
	VkPrimitiveTopology topology;
	VkPipeline pipeline;
	bool indexed;
	uint32_t indexCount;

	void createBuffers(const Engine::Mesh* mesh,
		const VkPhysicalDeviceMemoryProperties& memoryProperties);
	void createPipeline(
		const std::vector<VkPipelineShaderStageCreateInfo>& stages,
		VkViewport* viewport, VkRect2D* scissor, VkRenderPass renderPass,
		VkPipelineLayout pipelineLayout);
};

#endif
