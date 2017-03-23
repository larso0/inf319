#include "GLWindow.h"
#include "GLRenderer.h"
#include <stdexcept>

using namespace std;

GLWindow::GLWindow() :
handle(nullptr),
width(800),
height(600),
mouse({false, glm::vec2(), glm::vec2()}),
open(false),
renderer(nullptr)
{
}

GLWindow::~GLWindow() {
	close();
}

void GLWindow::init() {
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	handle = glfwCreateWindow(width, height, "Sandbox", nullptr, nullptr);
	if (!handle) {
		throw runtime_error("Failed to create window.");
	}

	glfwMakeContextCurrent(handle);

	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
		glfwDestroyWindow(handle);
		throw runtime_error("Failed to load OpenGL extensions.");
	}

	glfwSetWindowUserPointer(handle, this);
	glfwSetWindowSizeCallback(handle, windowSizeCallback);
	glfwSetKeyCallback(handle, keyCallback);
	glfwSetCursorPosCallback(handle, mousePositionCallback);
	double x, y;
	glfwGetCursorPos(handle, &x, &y);
	mouse.position = glm::vec2(x, y);

	glfwSwapInterval(1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(0.5f, 0.5f, 0.5f, 1.f);
	glViewport(0, 0, width, height);
	open = true;
}

void GLWindow::close() {
	if (!open) return;
	glfwDestroyWindow(handle);
}

bool GLWindow::shouldClose() const {
	return glfwWindowShouldClose(handle);
}

void GLWindow::resize(uint32_t w, uint32_t h) {
	width = w;
	height = h;
	if (handle) {
		glfwSetWindowSize(handle, w, h);
		glViewport(0, 0, w, h);
	}
}

glm::vec2 GLWindow::mouseMotion() {
	glm::vec2 motion = mouse.motion;
	mouse.motion = glm::vec2();
	return motion;
}

bool GLWindow::isCursorHidden() const {
	return mouse.hidden;
}

void GLWindow::toggleCursorHidden() {
	glfwSetInputMode(handle, GLFW_CURSOR,
		mouse.hidden ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
	mouse.hidden = !mouse.hidden;
}

Engine::KeyAction GLWindow::getKey(Engine::Key key) const {
	return static_cast<Engine::KeyAction>(glfwGetKey(handle,
		static_cast<int>(key)));
}

Engine::KeyAction GLWindow::getMouseButton(Engine::MouseButton btn) const {
	return static_cast<Engine::KeyAction>(glfwGetMouseButton(handle,
		static_cast<int>(btn)));
}

glm::vec2 GLWindow::getCursorPosition() const {
	return mouse.position;
}

uint32_t GLWindow::getWidth() const {
	return width;
}

uint32_t GLWindow::getHeight() const {
	return height;
}

Engine::Renderer& GLWindow::getRenderer() {
	if (!renderer) {
		renderer = new GLRenderer(*this);
	}
	return *renderer;
}

void GLWindow::windowSizeCallback(GLFWwindow* handle, int w, int h) {
	GLWindow& window = *((GLWindow*)glfwGetWindowUserPointer(handle));
	window.width = w;
	window.height = h;
	glViewport(0, 0, w, h);
}

void GLWindow::keyCallback(GLFWwindow* handle, int key, int, int action, int) {
	Window& window = *((Window*)glfwGetWindowUserPointer(handle));
	if (action != GLFW_RELEASE) return;
	switch(key) {
	case GLFW_KEY_ESCAPE:
		window.toggleCursorHidden();
		break;
	default:
		break;
	}
}

void GLWindow::mousePositionCallback(GLFWwindow* handle, double x, double y) {
	GLWindow& window = *((GLWindow*)glfwGetWindowUserPointer(handle));
	glm::vec2 position(x, y);
	glm::vec2 motion = position - window.mouse.position;
	if (window.mouse.hidden) {
		window.mouse.motion = position - window.mouse.position;
	}
	window.mouse.position = position;
}
