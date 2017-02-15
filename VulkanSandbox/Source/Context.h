#ifndef CONTEXT_H
#define CONTEXT_H

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

struct Context {
	GLFWwindow* window;
	VkInstance instance;
#ifndef NDEBUG
	 VkDebugReportCallbackEXT debugCallback;
#endif
};

#endif
