#ifndef GLWINDOW_H
#define GLWINDOW_H

#include <Engine/Window.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class GLWindow : public Engine::Window {
public:
	GLWindow();
	~GLWindow();

	void init() override;
	void close() override;
	bool shouldClose() const override;
	void resize(uint32_t w, uint32_t h) override;
	glm::vec2 mouseMotion() override;
	bool isCursorHidden() const override;
	void toggleCursorHidden() override;
	void handleEvents() override;
	Engine::KeyAction getKey(Engine::Key key) const override;
	Engine::KeyAction getMouseButton(Engine::MouseButton btn) const override;
	glm::vec2 getCursorPosition() const override;
	uint32_t getWidth() const override;
	uint32_t getHeight() const override;
	Engine::Renderer& getRenderer() override;

	void present() {
		glfwSwapBuffers(handle);
	}

private:
	GLFWwindow* handle;
	uint32_t width, height;

	struct Mouse {
		bool hidden;
		glm::vec2 position;
		glm::vec2 motion;
	} mouse;

	bool open;

	Engine::Renderer* renderer;

	bool haveResized;

	static void keyCallback(GLFWwindow* handle, int key, int, int action, int);
	static void charCallback(GLFWwindow* handle, unsigned int codepoint);
	static void mouseButtonCallback(GLFWwindow* handle, int button, int action, int mods);
	static void cursorPositionCallback(GLFWwindow* handle, double x, double y);
	static void cursorEnterCallback(GLFWwindow* handle, int entered);
	static void windowSizeCallback(GLFWwindow* window, int width, int height);
	static void fileDropCallback(GLFWwindow* handle, int count, const char** paths);
};

#endif
