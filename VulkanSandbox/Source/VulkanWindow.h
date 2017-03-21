#ifndef VULKANWINDOW_H_
#define VULKANWINDOW_H_

#include "VulkanContext.h"
#include "VulkanDevice.h"
#include <Engine/Window.h>
#include <Engine/Math.h>
#include <vector>

class VulkanRenderer;

class VulkanWindow : public Engine::Window {
	friend class VulkanRenderer;
public:
	VulkanWindow(VulkanContext& context);
	~VulkanWindow();

	void init() override;
	void close() override;

	void resize(uint32_t w, uint32_t h) override {
		viewport.width = w;
		viewport.height = h;
		scissor.extent.width = w;
		scissor.extent.height = h;
	}

	bool isCursorHidden() const {
		return mouse.hidden;
	}

	bool shouldClose() const {
		return glfwWindowShouldClose(handle);
	}

	bool getKey(int key) const {
		return glfwGetKey(handle, key);
	}

	glm::vec2 mouseMotion() {
		glm::vec2 motion = mouse.motion;
		mouse.motion = glm::vec2();
		return motion;
	}

	Engine::Renderer& getRenderer() override;

	virtual uint32_t getWidth() const override {
		return viewport.width;
	}

	virtual uint32_t getHeight() const override {
		return viewport.height;
	}

private:
	VulkanContext& context;
	GLFWwindow* handle;
	VkSurfaceKHR surface;
	VkViewport viewport;
	VkRect2D scissor;
	VulkanDevice* device;
	VkFormat colorFormat;
	VkSwapchainKHR swapchain;
	VkCommandBuffer presentCommandBuffer;
	std::vector<VkImage> presentImages;
	std::vector<VkImageView> presentImageViews;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	VkRenderPass renderPass;
	std::vector<VkFramebuffer> framebuffers;

	struct Mouse {
		bool hidden;
		glm::vec2 position;
		glm::vec2 motion;
	} mouse;

	bool open;

	VulkanRenderer* renderer;

	void createSwapchain();
	void setupSwapchainImages();
	void createDepthBuffer();
	void createRenderPass();
	void createFramebuffers();

	static void windowSizeCallback(GLFWwindow* window, int width, int height);
	static void keyCallback(GLFWwindow* handle, int key, int, int action, int);
	static void mousePositionCallback(GLFWwindow* handle, double x, double y);
};

#endif
