#ifndef VULKANWINDOW_H_
#define VULKANWINDOW_H_

#include "VulkanContext.h"
#include <Engine/Window.h>
#include <Engine/Math.h>
#include <vector>

class VulkanRenderer;

class VulkanWindow : public Engine::Window {
	friend class VulkanRenderer;
public:
	VulkanWindow(VulkanContext& context);
	~VulkanWindow();

	void resize(uint32_t w, uint32_t h) override {
		viewport.width = w;
		viewport.height = h;
		scissor.extent.width = w;
		scissor.extent.height = h;
	}

	Engine::Renderer& getRenderer() override;

	virtual uint32_t getWidth() const override {
		return viewport.width;
	}

	virtual uint32_t getHeight() const override {
		return viewport.height;
	}

//private:
	VulkanContext& context;
	GLFWwindow* handle;
	VkSurfaceKHR surface;
	VkViewport viewport;
	VkRect2D scissor;
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

	struct Mouse {
		bool hidden;
		glm::vec2 position;
		glm::vec2 motion;
		float sensitivity;
	} mouse;

	void createSwapchain();
	void createCommandPool();
	void setupSwapchainImages();
	void createDepthBuffer();
	void createRenderPass();
	void createFramebuffers();
};

#endif
