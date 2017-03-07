#ifndef VULKANWINDOW_H_
#define VULKANWINDOW_H_

#include "VulkanContext.h"
#include <Engine/Window.h>
#include <vector>

class VulkanRenderer;

class VulkanWindow : public Engine::Window {
	friend class VulkanRenderer;
public:
	VulkanWindow(VulkanContext& context);
	~VulkanWindow();

	void resize(uint32_t w, uint32_t h) override {
		width = w;
		height = h;
	}

	uint32_t getWidth() const override {
		return width;
	}

	uint32_t getHeight() const override {
		return height;
	}

	Engine::Renderer& getRenderer() override;

//private:
	VulkanContext& context;
	GLFWwindow* handle;
	VkSurfaceKHR surface;
	uint32_t width, height;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	uint32_t presentQueueIndex;
	VkFormat colorFormat;
	VkSwapchainKHR swapchain;
	VkCommandPool presentCommandPool;
	VkCommandBuffer presentCommandBuffer;
	std::vector<VkImage> presentImages;
	VkQueue presentQueue;
	std::vector<VkImageView> presentImageViews;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	VkRenderPass renderPass;
	std::vector<VkFramebuffer> framebuffers;

	void createSwapchain();
	void createCommandPool();
	void setupSwapchainImages();
	void createDepthBuffer();
	void createRenderPass();
	void createFramebuffers();
};

#endif
