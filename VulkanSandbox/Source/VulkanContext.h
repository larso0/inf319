#ifndef VULKANCONTEXT_H
#define VULKANCONTEXT_H

#include "VulkanIncludes.h"
#include <Engine/Context.h>
#include <vector>

class VulkanContext : public Engine::Context {
public:
	VulkanContext();
	~VulkanContext();

	Engine::Window& createWindow(int w, int h, int flags) override;

	VkInstance getInstance() const {
		return instance;
	}

	const std::vector<VkPhysicalDevice>& getPhysicalDevices() const {
		return physicalDevices;
	}

private:
	VkInstance instance;
#ifndef NDEBUG
	VkDebugReportCallbackEXT debugCallback;
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallback;
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback;
	void createDebugCallback();
#endif

	void loadExtensions();
	void createInstance();

	std::vector<VkPhysicalDevice> physicalDevices;
	std::vector<Engine::Window*> windows;
};

#endif
