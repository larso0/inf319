#ifndef VULKANWINDOW_H_
#define VULKANWINDOW_H_

#include "VulkanContext.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanImage.h"
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
		return glfwWindowShouldClose(handle) != 0;
	}

	void resize(uint32_t w, uint32_t h) override;

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

	void handleEvents() override {
		glfwPollEvents();
		if (haveResized) {
			updateSize();
		}
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
		return (uint32_t)viewport.width;
	}

	virtual uint32_t getHeight() const override {
		return (uint32_t)viewport.height;
	}

	Engine::Renderer& getRenderer() override;

private:
	VulkanContext& context;
	GLFWwindow* handle;
	VkSurfaceKHR surface;
	VkViewport viewport;
	VkRect2D scissor;
	VulkanDevice* device;
	VulkanSwapchain* swapchain;
	VkCommandBuffer presentCommandBuffer;
	VulkanImage* depthImage;
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

	void createDepthBuffer();
	void createRenderPass();
	void createFramebuffers();

	bool haveResized;
	void updateSize();

	static void keyCallback(GLFWwindow* handle, int key, int, int action, int);
	static void charCallback(GLFWwindow* handle, unsigned int codepoint);
	static void mouseButtonCallback(GLFWwindow* handle, int button, int action, int mods);
	static void cursorPositionCallback(GLFWwindow* handle, double x, double y);
	static void cursorEnterCallback(GLFWwindow* handle, int entered);
	static void windowSizeCallback(GLFWwindow* window, int width, int height);
	static void fileDropCallback(GLFWwindow* handle, int count, const char** paths);
};

#endif
