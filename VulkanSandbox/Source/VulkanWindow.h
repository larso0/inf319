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

	bool shouldClose() const override {
		return glfwWindowShouldClose(handle);
	}

	void resize(uint32_t w, uint32_t h) override {
		viewport.width = w;
		viewport.height = h;
		scissor.extent.width = w;
		scissor.extent.height = h;
	}

	glm::vec2 mouseMotion() override {
		glm::vec2 motion = mouse.motion;
		mouse.motion = glm::vec2();
		return motion;
	}

	bool isCursorHidden() const override {
		return mouse.hidden;
	}

	void toggleCursorHidden() override {
		glfwSetInputMode(handle, GLFW_CURSOR,
			mouse.hidden ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
		mouse.hidden = !mouse.hidden;
	}

	Engine::KeyAction getKey(Engine::Key key) const override {
		return static_cast<Engine::KeyAction>(glfwGetKey(handle,
			static_cast<int>(key)));
	}

	Engine::KeyAction getMouseButton(Engine::MouseButton btn) const override {
		return static_cast<Engine::KeyAction>(glfwGetMouseButton(handle,
			static_cast<int>(btn)));
	}

	glm::vec2 getCursorPosition() const override {
		return mouse.position;
	}

	virtual uint32_t getWidth() const override {
		return viewport.width;
	}

	virtual uint32_t getHeight() const override {
		return viewport.height;
	}

	Engine::Renderer& getRenderer() override;

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
