#ifndef CONTEXT_H
#define CONTEXT_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>

struct ContextOld {
	uint32_t width, height;
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkDevice device;
	uint32_t presentQueueIdx;
	VkSwapchainKHR swapchain;
	VkQueue presentQueue;
	VkCommandPool commandPool;
	VkCommandBuffer setupCmdBuffer;
	VkCommandBuffer drawCmdBuffer;
	std::vector<VkImage> swapchainImages;
	VkFormat colorFormat;
	std::vector<VkImageView> swapchainImageViews;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	VkRenderPass renderPass;
	std::vector<VkFramebuffer> framebuffers;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkShaderModule vertexShaderModule;
	VkShaderModule fragmentShaderModule;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
};

#endif
