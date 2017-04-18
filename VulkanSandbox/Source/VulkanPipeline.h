#ifndef VULKANPIPELINE_H
#define VULKANPIPELINE_H

#include <vulkan/vulkan.h>
#include "VulkanShaderProgram.h"

class VulkanPipeline {
public:
	VulkanPipeline(const VulkanShaderProgram& program, VkRenderPass renderPass,
		VkPipelineLayout pipelineLayout, VkPrimitiveTopology topology,
		uint32_t vertexAttributeCount,
		const VkVertexInputAttributeDescription* pVertexAttributes);
	~VulkanPipeline();

	VkPipeline getHandle() const {
		return handle;
	}
	
private:
	VkDevice device;
	VkPipeline handle;
};

#endif