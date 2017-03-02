#ifndef VULKANCONTEXT_H
#define VULKANCONTEXT_H

#include "VulkanIncludes.h"
#include <Engine/Context.h>
#include <vector>

class VulkanWindow;

class VulkanContext : public Engine::Context {
	friend class VulkanWindow;
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

	std::vector<VkPhysicalDevice> physicalDevices;
	std::vector<VkDevice> logicalDevices;
	void pickDevice(VkSurfaceKHR surface, VkPhysicalDevice* dstPhysicalDevice,
		VkDevice* dstDevice, uint32_t* dstPresentI);
};

#endif
