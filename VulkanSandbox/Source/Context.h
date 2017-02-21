#ifndef CONTEXT_H
#define CONTEXT_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

struct Context {
	int width, height;
	GLFWwindow* window;
	VkInstance instance;
#ifndef NDEBUG
	 VkDebugReportCallbackEXT debugCallback;
#endif
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
};

#endif
