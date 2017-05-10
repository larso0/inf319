#ifndef VULKANPIPELINE_H
#define VULKANPIPELINE_H

#include <vulkan/vulkan.h>
#include "VulkanShaderProgram.h"
#include <Engine/Vertex.h>

class VulkanPipeline {
public:
	VulkanPipeline(
		const VulkanShaderProgram& program,
		VkRenderPass renderPass,
		VkPipelineLayout pipelineLayout,
		VkPrimitiveTopology topology,
		uint32_t vertexAttributeCount,
		const VkVertexInputAttributeDescription* pVertexAttributes,
		uint32_t vertexStride = (uint32_t)Engine::Vertex::Stride
	);

	~VulkanPipeline();

	VkPipeline getHandle() const {
		return handle;
	}

private:
	VkDevice device;
	VkPipeline handle;
};

#endif
