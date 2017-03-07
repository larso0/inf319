#ifndef CONTEXT_H
#define CONTEXT_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>

struct ContextOld {
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
};

#endif
