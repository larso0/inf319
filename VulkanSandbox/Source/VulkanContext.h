#ifndef VULKANCONTEXT_H
#define VULKANCONTEXT_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <Engine/Context.h>

class VulkanContext : public Engine::Context {
public:
	VulkanContext();
	~VulkanContext();

	Engine::Window* createWindow(int w, int h, int flags) override;

//private:

	VkInstance instance;
#ifndef NDEBUG
	VkDebugReportCallbackEXT debugCallback;
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallback;
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback;
	void createDebugCallback();
#endif

	void loadExtensions();
	void createInstance();
};

#endif
